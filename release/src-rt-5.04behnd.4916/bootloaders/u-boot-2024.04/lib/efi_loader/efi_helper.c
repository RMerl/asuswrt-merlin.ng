// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2020, Linaro Limited
 */

#define LOG_CATEGORY LOGC_EFI
#include <bootm.h>
#include <env.h>
#include <image.h>
#include <log.h>
#include <malloc.h>
#include <mapmem.h>
#include <dm.h>
#include <fs.h>
#include <efi_api.h>
#include <efi_load_initrd.h>
#include <efi_loader.h>
#include <efi_variable.h>
#include <linux/libfdt.h>
#include <linux/list.h>

#if defined(CONFIG_CMD_EFIDEBUG) || defined(CONFIG_EFI_LOAD_FILE2_INITRD)
/* GUID used by Linux to identify the LoadFile2 protocol with the initrd */
const efi_guid_t efi_lf2_initrd_guid = EFI_INITRD_MEDIA_GUID;
#endif

/**
 * efi_create_current_boot_var() - Return Boot#### name were #### is replaced by
 *			           the value of BootCurrent
 *
 * @var_name:		variable name
 * @var_name_size:	size of var_name
 *
 * Return:	Status code
 */
static efi_status_t efi_create_current_boot_var(u16 var_name[],
						size_t var_name_size)
{
	efi_uintn_t boot_current_size;
	efi_status_t ret;
	u16 boot_current;
	u16 *pos;

	boot_current_size = sizeof(boot_current);
	ret = efi_get_variable_int(u"BootCurrent",
				   &efi_global_variable_guid, NULL,
				   &boot_current_size, &boot_current, NULL);
	if (ret != EFI_SUCCESS)
		goto out;

	pos = efi_create_indexed_name(var_name, var_name_size, "Boot",
				      boot_current);
	if (!pos) {
		ret = EFI_OUT_OF_RESOURCES;
		goto out;
	}

out:
	return ret;
}

/**
 * efi_get_dp_from_boot() - Retrieve and return a device path from an EFI
 *			    Boot### variable.
 *			    A boot option may contain an array of device paths.
 *			    We use a VenMedia() with a specific GUID to identify
 *			    the usage of the array members. This function is
 *			    used to extract a specific device path
 *
 * @guid:	vendor GUID of the VenMedia() device path node identifying the
 *		device path
 *
 * Return:	device path or NULL. Caller must free the returned value
 */
struct efi_device_path *efi_get_dp_from_boot(const efi_guid_t guid)
{
	struct efi_load_option lo;
	void *var_value;
	efi_uintn_t size;
	efi_status_t ret;
	u16 var_name[16];

	ret = efi_create_current_boot_var(var_name, sizeof(var_name));
	if (ret != EFI_SUCCESS)
		return NULL;

	var_value = efi_get_var(var_name, &efi_global_variable_guid, &size);
	if (!var_value)
		return NULL;

	ret = efi_deserialize_load_option(&lo, var_value, &size);
	if (ret != EFI_SUCCESS)
		goto err;

	return efi_dp_from_lo(&lo, &guid);

err:
	free(var_value);
	return NULL;
}

const struct guid_to_hash_map {
	efi_guid_t guid;
	const char algo[32];
	u32 bits;
} guid_to_hash[] = {
	{
		EFI_CERT_X509_SHA256_GUID,
		"sha256",
		SHA256_SUM_LEN * 8,
	},
	{
		EFI_CERT_SHA256_GUID,
		"sha256",
		SHA256_SUM_LEN * 8,
	},
	{
		EFI_CERT_X509_SHA384_GUID,
		"sha384",
		SHA384_SUM_LEN * 8,
	},
	{
		EFI_CERT_X509_SHA512_GUID,
		"sha512",
		SHA512_SUM_LEN * 8,
	},
};

#define MAX_GUID_TO_HASH_COUNT ARRAY_SIZE(guid_to_hash)

/** guid_to_sha_str - return the sha string e.g "sha256" for a given guid
 *                    used on EFI security databases
 *
 * @guid: guid to check
 *
 * Return: len or 0 if no match is found
 */
const char *guid_to_sha_str(const efi_guid_t *guid)
{
	size_t i;

	for (i = 0; i < MAX_GUID_TO_HASH_COUNT; i++) {
		if (!guidcmp(guid, &guid_to_hash[i].guid))
			return guid_to_hash[i].algo;
	}

	return NULL;
}

/** algo_to_len - return the sha size in bytes for a given string
 *
 * @algo: string indicating hashing algorithm to check
 *
 * Return: length of hash in bytes or 0 if no match is found
 */
int algo_to_len(const char *algo)
{
	size_t i;

	for (i = 0; i < MAX_GUID_TO_HASH_COUNT; i++) {
		if (!strcmp(algo, guid_to_hash[i].algo))
			return guid_to_hash[i].bits / 8;
	}

	return 0;
}

/** efi_link_dev - link the efi_handle_t and udevice
 *
 * @handle:	efi handle to associate with udevice
 * @dev:	udevice to associate with efi handle
 *
 * Return:	0 on success, negative on failure
 */
int efi_link_dev(efi_handle_t handle, struct udevice *dev)
{
	handle->dev = dev;
	return dev_tag_set_ptr(dev, DM_TAG_EFI, handle);
}

/**
 * efi_unlink_dev() - unlink udevice and handle
 *
 * @handle:	EFI handle to unlink
 *
 * Return:	0 on success, negative on failure
 */
int efi_unlink_dev(efi_handle_t handle)
{
	int ret;

	ret = dev_tag_del(handle->dev, DM_TAG_EFI);
	if (ret)
		return ret;
	handle->dev = NULL;

	return 0;
}

static int u16_tohex(u16 c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;

	/* not hexadecimal */
	return -1;
}

bool efi_varname_is_load_option(u16 *var_name16, int *index)
{
	int id, i, digit;

	if (memcmp(var_name16, u"Boot", 8))
		return false;

	for (id = 0, i = 0; i < 4; i++) {
		digit = u16_tohex(var_name16[4 + i]);
		if (digit < 0)
			break;
		id = (id << 4) + digit;
	}
	if (i == 4 && !var_name16[8]) {
		if (index)
			*index = id;
		return true;
	}

	return false;
}

/**
 * efi_next_variable_name() - get next variable name
 *
 * This function is a wrapper of efi_get_next_variable_name_int().
 * If efi_get_next_variable_name_int() returns EFI_BUFFER_TOO_SMALL,
 * @size and @buf are updated by new buffer size and realloced buffer.
 *
 * @size:	pointer to the buffer size
 * @buf:	pointer to the buffer
 * @guid:	pointer to the guid
 * Return:	status code
 */
efi_status_t efi_next_variable_name(efi_uintn_t *size, u16 **buf, efi_guid_t *guid)
{
	u16 *p;
	efi_status_t ret;
	efi_uintn_t buf_size = *size;

	ret = efi_get_next_variable_name_int(&buf_size, *buf, guid);
	if (ret == EFI_NOT_FOUND)
		return ret;
	if (ret == EFI_BUFFER_TOO_SMALL) {
		p = realloc(*buf, buf_size);
		if (!p)
			return EFI_OUT_OF_RESOURCES;

		*buf = p;
		*size = buf_size;
		ret = efi_get_next_variable_name_int(&buf_size, *buf, guid);
	}

	return ret;
}

/**
 * efi_search_bootorder() - search the boot option index in BootOrder
 *
 * @bootorder:	pointer to the BootOrder variable
 * @num:	number of BootOrder entry
 * @target:	target boot option index to search
 * @index:	pointer to store the index of BootOrder variable
 * Return:	true if exists, false otherwise
 */
bool efi_search_bootorder(u16 *bootorder, efi_uintn_t num, u32 target, u32 *index)
{
	u32 i;

	for (i = 0; i < num; i++) {
		if (target == bootorder[i]) {
			if (index)
				*index = i;

			return true;
		}
	}

	return false;
}

/**
 * efi_env_set_load_options() - set load options from environment variable
 *
 * @handle:		the image handle
 * @env_var:		name of the environment variable
 * @load_options:	pointer to load options (output)
 * Return:		status code
 */
efi_status_t efi_env_set_load_options(efi_handle_t handle,
				      const char *env_var,
				      u16 **load_options)
{
	const char *env = env_get(env_var);
	size_t size;
	u16 *pos;
	efi_status_t ret;

	*load_options = NULL;
	if (!env)
		return EFI_SUCCESS;
	size = sizeof(u16) * (utf8_utf16_strlen(env) + 1);
	pos = calloc(size, 1);
	if (!pos)
		return EFI_OUT_OF_RESOURCES;
	*load_options = pos;
	utf8_utf16_strcpy(&pos, env);
	ret = efi_set_load_options(handle, size, *load_options);
	if (ret != EFI_SUCCESS) {
		free(*load_options);
		*load_options = NULL;
	}
	return ret;
}

/**
 * copy_fdt() - Copy the device tree to a new location available to EFI
 *
 * The FDT is copied to a suitable location within the EFI memory map.
 * Additional 12 KiB are added to the space in case the device tree needs to be
 * expanded later with fdt_open_into().
 *
 * @fdtp:	On entry a pointer to the flattened device tree.
 *		On exit a pointer to the copy of the flattened device tree.
 *		FDT start
 * Return:	status code
 */
static efi_status_t copy_fdt(void **fdtp)
{
	unsigned long fdt_ram_start = -1L, fdt_pages;
	efi_status_t ret = 0;
	void *fdt, *new_fdt;
	u64 new_fdt_addr;
	uint fdt_size;
	int i;

	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		u64 ram_start = gd->bd->bi_dram[i].start;
		u64 ram_size = gd->bd->bi_dram[i].size;

		if (!ram_size)
			continue;

		if (ram_start < fdt_ram_start)
			fdt_ram_start = ram_start;
	}

	/*
	 * Give us at least 12 KiB of breathing room in case the device tree
	 * needs to be expanded later.
	 */
	fdt = *fdtp;
	fdt_pages = efi_size_in_pages(fdt_totalsize(fdt) + 0x3000);
	fdt_size = fdt_pages << EFI_PAGE_SHIFT;

	ret = efi_allocate_pages(EFI_ALLOCATE_ANY_PAGES,
				 EFI_ACPI_RECLAIM_MEMORY, fdt_pages,
				 &new_fdt_addr);
	if (ret != EFI_SUCCESS) {
		log_err("ERROR: Failed to reserve space for FDT\n");
		goto done;
	}
	new_fdt = (void *)(uintptr_t)new_fdt_addr;
	memcpy(new_fdt, fdt, fdt_totalsize(fdt));
	fdt_set_totalsize(new_fdt, fdt_size);

	*fdtp = (void *)(uintptr_t)new_fdt_addr;
done:
	return ret;
}

/**
 * efi_get_configuration_table() - get configuration table
 *
 * @guid:	GUID of the configuration table
 * Return:	pointer to configuration table or NULL
 */
void *efi_get_configuration_table(const efi_guid_t *guid)
{
	size_t i;

	for (i = 0; i < systab.nr_tables; i++) {
		if (!guidcmp(guid, &systab.tables[i].guid))
			return systab.tables[i].table;
	}
	return NULL;
}

/**
 * efi_install_fdt() - install device tree
 *
 * If fdt is not EFI_FDT_USE_INTERNAL, the device tree located at that memory
 * address will be installed as configuration table, otherwise the device
 * tree located at the address indicated by environment variable fdt_addr or as
 * fallback fdtcontroladdr will be used.
 *
 * On architectures using ACPI tables device trees shall not be installed as
 * configuration table.
 *
 * @fdt:	address of device tree or EFI_FDT_USE_INTERNAL to use
 *		the hardware device tree as indicated by environment variable
 *		fdt_addr or as fallback the internal device tree as indicated by
 *		the environment variable fdtcontroladdr
 * Return:	status code
 */
efi_status_t efi_install_fdt(void *fdt)
{
	struct bootm_headers img = { 0 };
	efi_status_t ret;

	/*
	 * The EBBR spec requires that we have either an FDT or an ACPI table
	 * but not both.
	 */
	if (CONFIG_IS_ENABLED(GENERATE_ACPI_TABLE) && fdt)
		log_warning("WARNING: Can't have ACPI table and device tree - ignoring DT.\n");

	if (fdt == EFI_FDT_USE_INTERNAL) {
		const char *fdt_opt;
		uintptr_t fdt_addr;

		/* Look for device tree that is already installed */
		if (efi_get_configuration_table(&efi_guid_fdt))
			return EFI_SUCCESS;
		/* Check if there is a hardware device tree */
		fdt_opt = env_get("fdt_addr");
		/* Use our own device tree as fallback */
		if (!fdt_opt) {
			fdt_opt = env_get("fdtcontroladdr");
			if (!fdt_opt) {
				log_err("ERROR: need device tree\n");
				return EFI_NOT_FOUND;
			}
		}
		fdt_addr = hextoul(fdt_opt, NULL);
		if (!fdt_addr) {
			log_err("ERROR: invalid $fdt_addr or $fdtcontroladdr\n");
			return EFI_LOAD_ERROR;
		}
		fdt = map_sysmem(fdt_addr, 0);
	}

	/* Install device tree */
	if (fdt_check_header(fdt)) {
		log_err("ERROR: invalid device tree\n");
		return EFI_LOAD_ERROR;
	}

	if (CONFIG_IS_ENABLED(GENERATE_ACPI_TABLE)) {
		/* Create memory reservations as indicated by the device tree */
		efi_carve_out_dt_rsv(fdt);
		return EFI_SUCCESS;
	}

	/* Prepare device tree for payload */
	ret = copy_fdt(&fdt);
	if (ret) {
		log_err("ERROR: out of memory\n");
		return EFI_OUT_OF_RESOURCES;
	}

	if (image_setup_libfdt(&img, fdt, NULL)) {
		log_err("ERROR: failed to process device tree\n");
		return EFI_LOAD_ERROR;
	}

	/* Create memory reservations as indicated by the device tree */
	efi_carve_out_dt_rsv(fdt);

	efi_try_purge_kaslr_seed(fdt);

	if (CONFIG_IS_ENABLED(EFI_TCG2_PROTOCOL_MEASURE_DTB)) {
		ret = efi_tcg2_measure_dtb(fdt);
		if (ret == EFI_SECURITY_VIOLATION) {
			log_err("ERROR: failed to measure DTB\n");
			return ret;
		}
	}

	/* Install device tree as UEFI table */
	ret = efi_install_configuration_table(&efi_guid_fdt, fdt);
	if (ret != EFI_SUCCESS) {
		log_err("ERROR: failed to install device tree\n");
		return ret;
	}

	return EFI_SUCCESS;
}

/**
 * do_bootefi_exec() - execute EFI binary
 *
 * The image indicated by @handle is started. When it returns the allocated
 * memory for the @load_options is freed.
 *
 * @handle:		handle of loaded image
 * @load_options:	load options
 * Return:		status code
 *
 * Load the EFI binary into a newly assigned memory unwinding the relocation
 * information, install the loaded image protocol, and call the binary.
 */
efi_status_t do_bootefi_exec(efi_handle_t handle, void *load_options)
{
	efi_status_t ret;
	efi_uintn_t exit_data_size = 0;
	u16 *exit_data = NULL;
	struct efi_event *evt;

	/* On ARM switch from EL3 or secure mode to EL2 or non-secure mode */
	switch_to_non_secure_mode();

	/*
	 * The UEFI standard requires that the watchdog timer is set to five
	 * minutes when invoking an EFI boot option.
	 *
	 * Unified Extensible Firmware Interface (UEFI), version 2.7 Errata A
	 * 7.5. Miscellaneous Boot Services - EFI_BOOT_SERVICES.SetWatchdogTimer
	 */
	ret = efi_set_watchdog(300);
	if (ret != EFI_SUCCESS) {
		log_err("ERROR: Failed to set watchdog timer\n");
		goto out;
	}

	/* Call our payload! */
	ret = EFI_CALL(efi_start_image(handle, &exit_data_size, &exit_data));
	if (ret != EFI_SUCCESS) {
		log_err("## Application failed, r = %lu\n",
			ret & ~EFI_ERROR_MASK);
		if (exit_data) {
			log_err("## %ls\n", exit_data);
			efi_free_pool(exit_data);
		}
	}

	efi_restore_gd();

out:
	free(load_options);

	if (IS_ENABLED(CONFIG_EFI_LOAD_FILE2_INITRD)) {
		if (efi_initrd_deregister() != EFI_SUCCESS)
			log_err("Failed to remove loadfile2 for initrd\n");
	}

	/* Notify EFI_EVENT_GROUP_RETURN_TO_EFIBOOTMGR event group. */
	list_for_each_entry(evt, &efi_events, link) {
		if (evt->group &&
		    !guidcmp(evt->group,
			     &efi_guid_event_group_return_to_efibootmgr)) {
			efi_signal_event(evt);
			EFI_CALL(systab.boottime->close_event(evt));
			break;
		}
	}

	/* Control is returned to U-Boot, disable EFI watchdog */
	efi_set_watchdog(0);

	return ret;
}
