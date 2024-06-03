// SPDX-License-Identifier: GPL-2.0+
/*
 *  EFI setup code
 *
 *  Copyright (c) 2016-2018 Alexander Graf et al.
 */

#include <common.h>
#include <bootm.h>
#include <efi_loader.h>

#define OBJ_LIST_NOT_INITIALIZED 1

static efi_status_t efi_obj_list_initialized = OBJ_LIST_NOT_INITIALIZED;

/*
 * Allow unaligned memory access.
 *
 * This routine is overridden by architectures providing this feature.
 */
void __weak allow_unaligned(void)
{
}

/**
 * efi_init_platform_lang() - define supported languages
 *
 * Set the PlatformLangCodes and PlatformLang variables.
 *
 * Return:	status code
 */
static efi_status_t efi_init_platform_lang(void)
{
	efi_status_t ret;
	efi_uintn_t data_size = 0;
	char *lang = CONFIG_EFI_PLATFORM_LANG_CODES;
	char *pos;

	/*
	 * Variable PlatformLangCodes defines the language codes that the
	 * machine can support.
	 */
	ret = EFI_CALL(efi_set_variable(L"PlatformLangCodes",
					&efi_global_variable_guid,
					EFI_VARIABLE_BOOTSERVICE_ACCESS |
					EFI_VARIABLE_RUNTIME_ACCESS,
					sizeof(CONFIG_EFI_PLATFORM_LANG_CODES),
					CONFIG_EFI_PLATFORM_LANG_CODES));
	if (ret != EFI_SUCCESS)
		goto out;

	/*
	 * Variable PlatformLang defines the language that the machine has been
	 * configured for.
	 */
	ret = EFI_CALL(efi_get_variable(L"PlatformLang",
					&efi_global_variable_guid,
					NULL, &data_size, &pos));
	if (ret == EFI_BUFFER_TOO_SMALL) {
		/* The variable is already set. Do not change it. */
		ret = EFI_SUCCESS;
		goto out;
	}

	/*
	 * The list of supported languages is semicolon separated. Use the first
	 * language to initialize PlatformLang.
	 */
	pos = strchr(lang, ';');
	if (pos)
		*pos = 0;

	ret = EFI_CALL(efi_set_variable(L"PlatformLang",
					&efi_global_variable_guid,
					EFI_VARIABLE_NON_VOLATILE |
					EFI_VARIABLE_BOOTSERVICE_ACCESS |
					EFI_VARIABLE_RUNTIME_ACCESS,
					1 + strlen(lang), lang));
out:
	if (ret != EFI_SUCCESS)
		printf("EFI: cannot initialize platform language settings\n");
	return ret;
}

/**
 * efi_init_obj_list() - Initialize and populate EFI object list
 *
 * Return:	status code
 */
efi_status_t efi_init_obj_list(void)
{
	u64 os_indications_supported = 0; /* None */
	efi_status_t ret = EFI_SUCCESS;

	/* Initialize once only */
	if (efi_obj_list_initialized != OBJ_LIST_NOT_INITIALIZED)
		return efi_obj_list_initialized;

	/* Allow unaligned memory access */
	allow_unaligned();

	/* On ARM switch from EL3 or secure mode to EL2 or non-secure mode */
	switch_to_non_secure_mode();

	/* Define supported languages */
	ret = efi_init_platform_lang();
	if (ret != EFI_SUCCESS)
		goto out;

	/* Indicate supported features */
	ret = EFI_CALL(efi_set_variable(L"OsIndicationsSupported",
					&efi_global_variable_guid,
					EFI_VARIABLE_BOOTSERVICE_ACCESS |
					EFI_VARIABLE_RUNTIME_ACCESS,
					sizeof(os_indications_supported),
					&os_indications_supported));
	if (ret != EFI_SUCCESS)
		goto out;

	/* Indicate supported runtime services */
	ret = efi_init_runtime_supported();
	if (ret != EFI_SUCCESS)
		goto out;

	/* Initialize system table */
	ret = efi_initialize_system_table();
	if (ret != EFI_SUCCESS)
		goto out;

	/* Initialize root node */
	ret = efi_root_node_register();
	if (ret != EFI_SUCCESS)
		goto out;

	/* Initialize EFI driver uclass */
	ret = efi_driver_init();
	if (ret != EFI_SUCCESS)
		goto out;

	ret = efi_console_register();
	if (ret != EFI_SUCCESS)
		goto out;
#ifdef CONFIG_PARTITIONS
	ret = efi_disk_register();
	if (ret != EFI_SUCCESS)
		goto out;
#endif
#if defined(CONFIG_LCD) || defined(CONFIG_DM_VIDEO)
	ret = efi_gop_register();
	if (ret != EFI_SUCCESS)
		goto out;
#endif
#ifdef CONFIG_NET
	ret = efi_net_register();
	if (ret != EFI_SUCCESS)
		goto out;
#endif
#ifdef CONFIG_GENERATE_ACPI_TABLE
	ret = efi_acpi_register();
	if (ret != EFI_SUCCESS)
		goto out;
#endif
#ifdef CONFIG_GENERATE_SMBIOS_TABLE
	ret = efi_smbios_register();
	if (ret != EFI_SUCCESS)
		goto out;
#endif
	ret = efi_watchdog_register();
	if (ret != EFI_SUCCESS)
		goto out;

	/* Initialize EFI runtime services */
	ret = efi_reset_system_init();
	if (ret != EFI_SUCCESS)
		goto out;

out:
	efi_obj_list_initialized = ret;
	return ret;
}
