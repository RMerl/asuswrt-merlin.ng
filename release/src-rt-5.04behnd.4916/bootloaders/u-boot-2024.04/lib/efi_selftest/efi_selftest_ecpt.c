// SPDX-License-Identifier: GPL-2.0+
/*
 * efi_selftest_fdt
 *
 * Copyright (c) 2022 Heinrich Schuchardt <xypron.glpk@gmx.de>
 *
 * Check the EFI_CONFORMANCE_PROFILE_TABLE
 */

#include <efi_selftest.h>

static const efi_guid_t guid_ecpt = EFI_CONFORMANCE_PROFILES_TABLE_GUID;
static const efi_guid_t guid_ebbr_2_1 = EFI_CONFORMANCE_PROFILE_EBBR_2_1_GUID;

/*
 * ecpt_find_guid() - find GUID in EFI Conformance Profile Table
 *
 * @ecpt:	EFI Conformance Profile Table
 * @guid:	GUID to find
 * Return:	EFI_ST_SUCCESS for success
 */
static int ecpt_find_guid(struct efi_conformance_profiles_table *ecpt,
			  const efi_guid_t *guid) {
	int i;

	for (i = 0; i < ecpt->number_of_profiles; ++i) {
		if (!memcmp(&ecpt->conformance_profiles[i], guid, 16))
			return EFI_ST_SUCCESS;
	}
	efi_st_error("GUID %pU not found\n", guid);
	return EFI_ST_FAILURE;
}

/*
 * Execute unit test.
 *
 * Return:	EFI_ST_SUCCESS for success
 */
static int execute(void)
{
	struct efi_conformance_profiles_table *ecpt;
	int expected_entries = 0;

	ecpt = efi_st_get_config_table(&guid_ecpt);

	if (!ecpt) {
		efi_st_error("Missing EFI Conformance Profile Table\n");
		return EFI_ST_FAILURE;
	}

	if (ecpt->version != EFI_CONFORMANCE_PROFILES_TABLE_VERSION) {
		efi_st_error("Wrong table version\n");
		return EFI_ST_FAILURE;
	}

	if (CONFIG_IS_ENABLED(EFI_EBBR_2_1_CONFORMANCE)) {
		++expected_entries;
		if (ecpt_find_guid(ecpt, &guid_ebbr_2_1))
			return EFI_ST_FAILURE;
	}

	if (ecpt->number_of_profiles != expected_entries) {
		efi_st_error("Expected %d entries, found %d\n",
			     expected_entries, ecpt->number_of_profiles);
		return EFI_ST_FAILURE;
	}

	return EFI_ST_SUCCESS;
}


EFI_UNIT_TEST(ecpt) = {
	.name = "conformance profile table",
	.phase = EFI_EXECUTE_BEFORE_BOOTTIME_EXIT,
	.execute = execute,
};
