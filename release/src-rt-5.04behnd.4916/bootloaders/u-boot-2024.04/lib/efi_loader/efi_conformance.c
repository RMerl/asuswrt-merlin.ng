// SPDX-License-Identifier: GPL-2.0-only
/*
 *  EFI conformance profile table
 *
 *  Copyright (C) 2022 Arm Ltd.
 */

#include <efi_loader.h>
#include <log.h>
#include <efi_api.h>
#include <malloc.h>

static const efi_guid_t efi_ecpt_guid = EFI_CONFORMANCE_PROFILES_TABLE_GUID;
static const efi_guid_t efi_ebbr_2_1_guid =
	EFI_CONFORMANCE_PROFILE_EBBR_2_1_GUID;

/**
 * efi_ecpt_register() - Install the ECPT system table.
 *
 * Return: status code
 */
efi_status_t efi_ecpt_register(void)
{
	u16 num_entries = 0;
	struct efi_conformance_profiles_table *ecpt;
	efi_status_t ret;
	size_t ecpt_size;

	ecpt_size = num_entries * sizeof(efi_guid_t)
		+ sizeof(struct efi_conformance_profiles_table);
	ret = efi_allocate_pool(EFI_BOOT_SERVICES_DATA, ecpt_size,
				(void **)&ecpt);

	if (ret != EFI_SUCCESS) {
		log_err("Out of memory\n");

		return ret;
	}

	if (CONFIG_IS_ENABLED(EFI_EBBR_2_1_CONFORMANCE))
		guidcpy(&ecpt->conformance_profiles[num_entries++],
			&efi_ebbr_2_1_guid);

	ecpt->version = EFI_CONFORMANCE_PROFILES_TABLE_VERSION;
	ecpt->number_of_profiles = num_entries;

	/* Install the ECPT in the system configuration table. */
	ret = efi_install_configuration_table(&efi_ecpt_guid, (void *)ecpt);
	if (ret != EFI_SUCCESS) {
		log_err("Failed to install ECPT\n");
		efi_free_pool(ecpt);

		return ret;
	}

	log_debug("ECPT created\n");

	return EFI_SUCCESS;
}
