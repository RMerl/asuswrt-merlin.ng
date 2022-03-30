// SPDX-License-Identifier: GPL-2.0+
/*
 *  EFI application loader
 *
 *  Copyright (c) 2016 Alexander Graf
 */

#include <common.h>
#include <charset.h>
#include <command.h>
#include <dm.h>
#include <efi_loader.h>
#include <efi_selftest.h>
#include <errno.h>
#include <linux/libfdt.h>
#include <linux/libfdt_env.h>
#include <mapmem.h>
#include <memalign.h>
#include <asm-generic/sections.h>
#include <linux/linkage.h>

DECLARE_GLOBAL_DATA_PTR;

static struct efi_device_path *bootefi_image_path;
static struct efi_device_path *bootefi_device_path;

/*
 * Set the load options of an image from an environment variable.
 *
 * @handle:	the image handle
 * @env_var:	name of the environment variable
 * Return:	status code
 */
static efi_status_t set_load_options(efi_handle_t handle, const char *env_var)
{
	struct efi_loaded_image *loaded_image_info;
	size_t size;
	const char *env = env_get(env_var);
	u16 *pos;
	efi_status_t ret;

	ret = EFI_CALL(systab.boottime->open_protocol(
					handle,
					&efi_guid_loaded_image,
					(void **)&loaded_image_info,
					efi_root, NULL,
					EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL));
	if (ret != EFI_SUCCESS)
		return EFI_INVALID_PARAMETER;

	loaded_image_info->load_options = NULL;
	loaded_image_info->load_options_size = 0;
	if (!env)
		goto out;

	size = utf8_utf16_strlen(env) + 1;
	loaded_image_info->load_options = calloc(size, sizeof(u16));
	if (!loaded_image_info->load_options) {
		printf("ERROR: Out of memory\n");
		EFI_CALL(systab.boottime->close_protocol(handle,
							 &efi_guid_loaded_image,
							 efi_root, NULL));
		return EFI_OUT_OF_RESOURCES;
	}
	pos = loaded_image_info->load_options;
	utf8_utf16_strcpy(&pos, env);
	loaded_image_info->load_options_size = size * 2;

out:
	return EFI_CALL(systab.boottime->close_protocol(handle,
							&efi_guid_loaded_image,
							efi_root, NULL));
}

#if !CONFIG_IS_ENABLED(GENERATE_ACPI_TABLE)

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

	/*
	 * Safe fdt location is at 127 MiB.
	 * On the sandbox convert from the sandbox address space.
	 */
	new_fdt_addr = (uintptr_t)map_sysmem(fdt_ram_start + 0x7f00000 +
					     fdt_size, 0);
	ret = efi_allocate_pages(EFI_ALLOCATE_MAX_ADDRESS,
				 EFI_BOOT_SERVICES_DATA, fdt_pages,
				 &new_fdt_addr);
	if (ret != EFI_SUCCESS) {
		/* If we can't put it there, put it somewhere */
		new_fdt_addr = (ulong)memalign(EFI_PAGE_SIZE, fdt_size);
		ret = efi_allocate_pages(EFI_ALLOCATE_MAX_ADDRESS,
					 EFI_BOOT_SERVICES_DATA, fdt_pages,
					 &new_fdt_addr);
		if (ret != EFI_SUCCESS) {
			printf("ERROR: Failed to reserve space for FDT\n");
			goto done;
		}
	}
	new_fdt = (void *)(uintptr_t)new_fdt_addr;
	memcpy(new_fdt, fdt, fdt_totalsize(fdt));
	fdt_set_totalsize(new_fdt, fdt_size);

	*fdtp = (void *)(uintptr_t)new_fdt_addr;
done:
	return ret;
}

/*
 * efi_carve_out_dt_rsv() - Carve out DT reserved memory ranges
 *
 * The mem_rsv entries of the FDT are added to the memory map. Any failures are
 * ignored because this is not critical and we would rather continue to try to
 * boot.
 *
 * @fdt: Pointer to device tree
 */
static void efi_carve_out_dt_rsv(void *fdt)
{
	int nr_rsv, i;
	uint64_t addr, size, pages;

	nr_rsv = fdt_num_mem_rsv(fdt);

	/* Look for an existing entry and add it to the efi mem map. */
	for (i = 0; i < nr_rsv; i++) {
		if (fdt_get_mem_rsv(fdt, i, &addr, &size) != 0)
			continue;

		/* Convert from sandbox address space. */
		addr = (uintptr_t)map_sysmem(addr, 0);

		pages = efi_size_in_pages(size + (addr & EFI_PAGE_MASK));
		addr &= ~EFI_PAGE_MASK;
		if (!efi_add_memory_map(addr, pages, EFI_RESERVED_MEMORY_TYPE,
					false))
			printf("FDT memrsv map %d: Failed to add to map\n", i);
	}
}

/**
 * get_config_table() - get configuration table
 *
 * @guid:	GUID of the configuration table
 * Return:	pointer to configuration table or NULL
 */
static void *get_config_table(const efi_guid_t *guid)
{
	size_t i;

	for (i = 0; i < systab.nr_tables; i++) {
		if (!guidcmp(guid, &systab.tables[i].guid))
			return systab.tables[i].table;
	}
	return NULL;
}

#endif /* !CONFIG_IS_ENABLED(GENERATE_ACPI_TABLE) */

/**
 * efi_install_fdt() - install fdt passed by a command argument
 *
 * If fdt_opt is available, the device tree located at that memory address will
 * will be installed as configuration table, otherwise the device tree located
 * at the address indicated by environment variable fdtcontroladdr will be used.
 *
 * On architectures (x86) using ACPI tables device trees shall not be installed
 * as configuration table.
 *
 * @fdt_opt:	pointer to argument
 * Return:	status code
 */
static efi_status_t efi_install_fdt(const char *fdt_opt)
{
	/*
	 * The EBBR spec requires that we have either an FDT or an ACPI table
	 * but not both.
	 */
#if CONFIG_IS_ENABLED(GENERATE_ACPI_TABLE)
	if (fdt_opt) {
		printf("ERROR: can't have ACPI table and device tree.\n");
		return EFI_LOAD_ERROR;
	}
#else
	unsigned long fdt_addr;
	void *fdt;
	bootm_headers_t img = { 0 };
	efi_status_t ret;

	if (fdt_opt) {
		fdt_addr = simple_strtoul(fdt_opt, NULL, 16);
		if (!fdt_addr)
			return EFI_INVALID_PARAMETER;
	} else {
		/* Look for device tree that is already installed */
		if (get_config_table(&efi_guid_fdt))
			return EFI_SUCCESS;
		/* Use our own device tree as default */
		fdt_opt = env_get("fdtcontroladdr");
		if (!fdt_opt) {
			printf("ERROR: need device tree\n");
			return EFI_NOT_FOUND;
		}
		fdt_addr = simple_strtoul(fdt_opt, NULL, 16);
		if (!fdt_addr) {
			printf("ERROR: invalid $fdtcontroladdr\n");
			return EFI_LOAD_ERROR;
		}
	}

	/* Install device tree */
	fdt = map_sysmem(fdt_addr, 0);
	if (fdt_check_header(fdt)) {
		printf("ERROR: invalid device tree\n");
		return EFI_LOAD_ERROR;
	}

	/* Create memory reservations as indicated by the device tree */
	efi_carve_out_dt_rsv(fdt);

	/* Prepare device tree for payload */
	ret = copy_fdt(&fdt);
	if (ret) {
		printf("ERROR: out of memory\n");
		return EFI_OUT_OF_RESOURCES;
	}

	if (image_setup_libfdt(&img, fdt, 0, NULL)) {
		printf("ERROR: failed to process device tree\n");
		return EFI_LOAD_ERROR;
	}

	/* Install device tree as UEFI table */
	ret = efi_install_configuration_table(&efi_guid_fdt, fdt);
	if (ret != EFI_SUCCESS) {
		printf("ERROR: failed to install device tree\n");
		return ret;
	}
#endif /* GENERATE_ACPI_TABLE */

	return EFI_SUCCESS;
}

/**
 * do_bootefi_exec() - execute EFI binary
 *
 * @handle:		handle of loaded image
 * Return:		status code
 *
 * Load the EFI binary into a newly assigned memory unwinding the relocation
 * information, install the loaded image protocol, and call the binary.
 */
static efi_status_t do_bootefi_exec(efi_handle_t handle)
{
	efi_status_t ret;
	efi_uintn_t exit_data_size = 0;
	u16 *exit_data = NULL;

	/* Transfer environment variable as load options */
	ret = set_load_options(handle, "bootargs");
	if (ret != EFI_SUCCESS)
		return ret;

	/* Call our payload! */
	ret = EFI_CALL(efi_start_image(handle, &exit_data_size, &exit_data));
	printf("## Application terminated, r = %lu\n", ret & ~EFI_ERROR_MASK);
	if (ret && exit_data) {
		printf("## %ls\n", exit_data);
		efi_free_pool(exit_data);
	}

	efi_restore_gd();

	/*
	 * FIXME: Who is responsible for
	 *	free(loaded_image_info->load_options);
	 * Once efi_exit() is implemented correctly,
	 * handle itself doesn't exist here.
	 */

	return ret;
}

/**
 * do_efibootmgr() - execute EFI boot manager
 *
 * Return:	status code
 */
static int do_efibootmgr(void)
{
	efi_handle_t handle;
	efi_status_t ret;

	ret = efi_bootmgr_load(&handle);
	if (ret != EFI_SUCCESS) {
		printf("EFI boot manager: Cannot load any image\n");
		return CMD_RET_FAILURE;
	}

	ret = do_bootefi_exec(handle);

	if (ret != EFI_SUCCESS)
		return CMD_RET_FAILURE;

	return CMD_RET_SUCCESS;
}

/*
 * do_bootefi_image() - execute EFI binary
 *
 * Set up memory image for the binary to be loaded, prepare device path, and
 * then call do_bootefi_exec() to execute it.
 *
 * @image_opt:	string of image start address
 * Return:	status code
 */
static int do_bootefi_image(const char *image_opt)
{
	void *image_buf;
	struct efi_device_path *device_path, *image_path;
	struct efi_device_path *file_path = NULL;
	unsigned long addr, size;
	const char *size_str;
	efi_handle_t mem_handle = NULL, handle;
	efi_status_t ret;

#ifdef CONFIG_CMD_BOOTEFI_HELLO
	if (!strcmp(image_opt, "hello")) {
		char *saddr;

		saddr = env_get("loadaddr");
		size = __efi_helloworld_end - __efi_helloworld_begin;

		if (saddr)
			addr = simple_strtoul(saddr, NULL, 16);
		else
			addr = CONFIG_SYS_LOAD_ADDR;

		image_buf = map_sysmem(addr, size);
		memcpy(image_buf, __efi_helloworld_begin, size);

		device_path = NULL;
		image_path = NULL;
	} else
#endif
	{
		size_str = env_get("filesize");
		if (size_str)
			size = simple_strtoul(size_str, NULL, 16);
		else
			size = 0;

		addr = simple_strtoul(image_opt, NULL, 16);
		/* Check that a numeric value was passed */
		if (!addr && *image_opt != '0')
			return CMD_RET_USAGE;

		image_buf = map_sysmem(addr, size);

		device_path = bootefi_device_path;
		image_path = bootefi_image_path;
	}

	if (!device_path && !image_path) {
		/*
		 * Special case for efi payload not loaded from disk,
		 * such as 'bootefi hello' or for example payload
		 * loaded directly into memory via JTAG, etc:
		 */
		file_path = efi_dp_from_mem(EFI_RESERVED_MEMORY_TYPE,
					    (uintptr_t)image_buf, size);
		/*
		 * Make sure that device for device_path exist
		 * in load_image(). Otherwise, shell and grub will fail.
		 */
		ret = efi_create_handle(&mem_handle);
		if (ret != EFI_SUCCESS)
			goto out;

		ret = efi_add_protocol(mem_handle, &efi_guid_device_path,
				       file_path);
		if (ret != EFI_SUCCESS)
			goto out;
	} else {
		assert(device_path && image_path);
		file_path = efi_dp_append(device_path, image_path);
	}

	ret = EFI_CALL(efi_load_image(false, efi_root,
				      file_path, image_buf, size, &handle));
	if (ret != EFI_SUCCESS)
		goto out;

	ret = do_bootefi_exec(handle);

out:
	if (mem_handle)
		efi_delete_handle(mem_handle);
	if (file_path)
		efi_free_pool(file_path);

	if (ret != EFI_SUCCESS)
		return CMD_RET_FAILURE;

	return CMD_RET_SUCCESS;
}

#ifdef CONFIG_CMD_BOOTEFI_SELFTEST
static efi_status_t bootefi_run_prepare(const char *load_options_path,
		struct efi_device_path *device_path,
		struct efi_device_path *image_path,
		struct efi_loaded_image_obj **image_objp,
		struct efi_loaded_image **loaded_image_infop)
{
	efi_status_t ret;

	ret = efi_setup_loaded_image(device_path, image_path, image_objp,
				     loaded_image_infop);
	if (ret != EFI_SUCCESS)
		return ret;

	/* Transfer environment variable as load options */
	return set_load_options((efi_handle_t)*image_objp, load_options_path);
}

/**
 * bootefi_test_prepare() - prepare to run an EFI test
 *
 * Prepare to run a test as if it were provided by a loaded image.
 *
 * @image_objp:		pointer to be set to the loaded image handle
 * @loaded_image_infop:	pointer to be set to the loaded image protocol
 * @path:		dummy file path used to construct the device path
 *			set in the loaded image protocol
 * @load_options_path:	name of a U-Boot environment variable. Its value is
 *			set as load options in the loaded image protocol.
 * Return:		status code
 */
static efi_status_t bootefi_test_prepare
		(struct efi_loaded_image_obj **image_objp,
		 struct efi_loaded_image **loaded_image_infop, const char *path,
		 const char *load_options_path)
{
	efi_status_t ret;

	/* Construct a dummy device path */
	bootefi_device_path = efi_dp_from_mem(EFI_RESERVED_MEMORY_TYPE, 0, 0);
	if (!bootefi_device_path)
		return EFI_OUT_OF_RESOURCES;

	bootefi_image_path = efi_dp_from_file(NULL, 0, path);
	if (!bootefi_image_path) {
		ret = EFI_OUT_OF_RESOURCES;
		goto failure;
	}

	ret = bootefi_run_prepare(load_options_path, bootefi_device_path,
				  bootefi_image_path, image_objp,
				  loaded_image_infop);
	if (ret == EFI_SUCCESS)
		return ret;

	efi_free_pool(bootefi_image_path);
	bootefi_image_path = NULL;
failure:
	efi_free_pool(bootefi_device_path);
	bootefi_device_path = NULL;
	return ret;
}

/**
 * bootefi_run_finish() - finish up after running an EFI test
 *
 * @loaded_image_info: Pointer to a struct which holds the loaded image info
 * @image_obj: Pointer to a struct which holds the loaded image object
 */
static void bootefi_run_finish(struct efi_loaded_image_obj *image_obj,
			       struct efi_loaded_image *loaded_image_info)
{
	efi_restore_gd();
	free(loaded_image_info->load_options);
	efi_delete_handle(&image_obj->header);
}

/**
 * do_efi_selftest() - execute EFI selftest
 *
 * Return:	status code
 */
static int do_efi_selftest(void)
{
	struct efi_loaded_image_obj *image_obj;
	struct efi_loaded_image *loaded_image_info;
	efi_status_t ret;

	ret = bootefi_test_prepare(&image_obj, &loaded_image_info,
				   "\\selftest", "efi_selftest");
	if (ret != EFI_SUCCESS)
		return CMD_RET_FAILURE;

	/* Execute the test */
	ret = EFI_CALL(efi_selftest(&image_obj->header, &systab));
	bootefi_run_finish(image_obj, loaded_image_info);

	return ret != EFI_SUCCESS;
}
#endif /* CONFIG_CMD_BOOTEFI_SELFTEST */

/**
 * do_bootefi() - execute `bootefi` command
 *
 * @cmdtp:	table entry describing command
 * @flag:	bitmap indicating how the command was invoked
 * @argc:	number of arguments
 * @argv:	command line arguments
 * Return:	status code
 */
static int do_bootefi(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	efi_status_t ret;

	if (argc < 2)
		return CMD_RET_USAGE;

	/* Initialize EFI drivers */
	ret = efi_init_obj_list();
	if (ret != EFI_SUCCESS) {
		printf("Error: Cannot initialize UEFI sub-system, r = %lu\n",
		       ret & ~EFI_ERROR_MASK);
		return CMD_RET_FAILURE;
	}

	ret = efi_install_fdt(argc > 2 ? argv[2] : NULL);
	if (ret == EFI_INVALID_PARAMETER)
		return CMD_RET_USAGE;
	else if (ret != EFI_SUCCESS)
		return CMD_RET_FAILURE;

	if (!strcmp(argv[1], "bootmgr"))
		return do_efibootmgr();
#ifdef CONFIG_CMD_BOOTEFI_SELFTEST
	else if (!strcmp(argv[1], "selftest"))
		return do_efi_selftest();
#endif

	return do_bootefi_image(argv[1]);
}

#ifdef CONFIG_SYS_LONGHELP
static char bootefi_help_text[] =
	"<image address> [fdt address]\n"
	"  - boot EFI payload stored at address <image address>.\n"
	"    If specified, the device tree located at <fdt address> gets\n"
	"    exposed as EFI configuration table.\n"
#ifdef CONFIG_CMD_BOOTEFI_HELLO
	"bootefi hello\n"
	"  - boot a sample Hello World application stored within U-Boot\n"
#endif
#ifdef CONFIG_CMD_BOOTEFI_SELFTEST
	"bootefi selftest [fdt address]\n"
	"  - boot an EFI selftest application stored within U-Boot\n"
	"    Use environment variable efi_selftest to select a single test.\n"
	"    Use 'setenv efi_selftest list' to enumerate all tests.\n"
#endif
	"bootefi bootmgr [fdt address]\n"
	"  - load and boot EFI payload based on BootOrder/BootXXXX variables.\n"
	"\n"
	"    If specified, the device tree located at <fdt address> gets\n"
	"    exposed as EFI configuration table.\n";
#endif

U_BOOT_CMD(
	bootefi, 3, 0, do_bootefi,
	"Boots an EFI payload from memory",
	bootefi_help_text
);

void efi_set_bootdev(const char *dev, const char *devnr, const char *path)
{
	struct efi_device_path *device, *image;
	efi_status_t ret;

	/* efi_set_bootdev is typically called repeatedly, recover memory */
	efi_free_pool(bootefi_device_path);
	efi_free_pool(bootefi_image_path);

	ret = efi_dp_from_name(dev, devnr, path, &device, &image);
	if (ret == EFI_SUCCESS) {
		bootefi_device_path = device;
		if (image) {
			/* FIXME: image should not contain device */
			struct efi_device_path *image_tmp = image;

			efi_dp_split_file_path(image, &device, &image);
			efi_free_pool(image_tmp);
		}
		bootefi_image_path = image;
	} else {
		bootefi_device_path = NULL;
		bootefi_image_path = NULL;
	}
}
