// SPDX-License-Identifier: GPL-2.0+
/*
 *  EFI application loader
 *
 *  Copyright (c) 2016 Alexander Graf
 */

#define LOG_CATEGORY LOGC_EFI

#include <command.h>
#include <efi.h>
#include <efi_loader.h>
#include <exports.h>
#include <log.h>
#include <malloc.h>
#include <mapmem.h>
#include <vsprintf.h>
#include <asm-generic/sections.h>
#include <asm/global_data.h>
#include <linux/string.h>

DECLARE_GLOBAL_DATA_PTR;

static struct efi_device_path *test_image_path;
static struct efi_device_path *test_device_path;

static efi_status_t bootefi_run_prepare(const char *load_options_path,
		struct efi_device_path *device_path,
		struct efi_device_path *image_path,
		struct efi_loaded_image_obj **image_objp,
		struct efi_loaded_image **loaded_image_infop)
{
	efi_status_t ret;
	u16 *load_options;

	ret = efi_setup_loaded_image(device_path, image_path, image_objp,
				     loaded_image_infop);
	if (ret != EFI_SUCCESS)
		return ret;

	/* Transfer environment variable as load options */
	return efi_env_set_load_options((efi_handle_t)*image_objp,
					load_options_path,
					&load_options);
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
	test_device_path = efi_dp_from_mem(EFI_RESERVED_MEMORY_TYPE, 0, 0);
	if (!test_device_path)
		return EFI_OUT_OF_RESOURCES;

	test_image_path = efi_dp_from_file(NULL, path);
	if (!test_image_path) {
		ret = EFI_OUT_OF_RESOURCES;
		goto failure;
	}

	ret = bootefi_run_prepare(load_options_path, test_device_path,
				  test_image_path, image_objp,
				  loaded_image_infop);
	if (ret == EFI_SUCCESS)
		return ret;

failure:
	efi_free_pool(test_device_path);
	efi_free_pool(test_image_path);
	/* TODO: not sure calling clear function is necessary */
	efi_clear_bootdev();
	return ret;
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
	efi_restore_gd();
	free(loaded_image_info->load_options);
	efi_free_pool(test_device_path);
	efi_free_pool(test_image_path);
	if (ret != EFI_SUCCESS)
		efi_delete_handle(&image_obj->header);
	else
		ret = efi_delete_handle(&image_obj->header);

	return ret != EFI_SUCCESS;
}

/**
 * do_bootefi() - execute `bootefi` command
 *
 * @cmdtp:	table entry describing command
 * @flag:	bitmap indicating how the command was invoked
 * @argc:	number of arguments
 * @argv:	command line arguments
 * Return:	status code
 */
static int do_bootefi(struct cmd_tbl *cmdtp, int flag, int argc,
		      char *const argv[])
{
	efi_status_t ret;
	char *p;
	void *fdt, *image_buf;
	unsigned long addr, size;
	void *image_addr;
	size_t image_size;

	if (argc < 2)
		return CMD_RET_USAGE;

	if (argc > 2) {
		uintptr_t fdt_addr;

		fdt_addr = hextoul(argv[2], NULL);
		fdt = map_sysmem(fdt_addr, 0);
	} else {
		fdt = EFI_FDT_USE_INTERNAL;
	}

	if (IS_ENABLED(CONFIG_CMD_BOOTEFI_BOOTMGR) &&
	    !strcmp(argv[1], "bootmgr")) {
		ret = efi_bootmgr_run(fdt);

		if (ret != EFI_SUCCESS)
			return CMD_RET_FAILURE;

		return CMD_RET_SUCCESS;
	}

	if (IS_ENABLED(CONFIG_CMD_BOOTEFI_SELFTEST) &&
	    !strcmp(argv[1], "selftest")) {
		/* Initialize EFI drivers */
		ret = efi_init_obj_list();
		if (ret != EFI_SUCCESS) {
			log_err("Error: Cannot initialize UEFI sub-system, r = %lu\n",
				ret & ~EFI_ERROR_MASK);
			return CMD_RET_FAILURE;
		}

		ret = efi_install_fdt(fdt);
		if (ret != EFI_SUCCESS)
			return CMD_RET_FAILURE;

		return do_efi_selftest();
	}

	if (!IS_ENABLED(CONFIG_CMD_BOOTEFI_BINARY))
		return CMD_RET_SUCCESS;

	if (IS_ENABLED(CONFIG_CMD_BOOTEFI_HELLO) &&
	    !strcmp(argv[1], "hello")) {
		image_buf = __efi_helloworld_begin;
		size = __efi_helloworld_end - __efi_helloworld_begin;
		/* TODO: not sure calling clear function is necessary */
		efi_clear_bootdev();
	} else {
		addr = strtoul(argv[1], NULL, 16);
		/* Check that a numeric value was passed */
		if (!addr)
			return CMD_RET_USAGE;
		image_buf = map_sysmem(addr, 0);

		p  = strchr(argv[1], ':');
		if (p) {
			size = strtoul(++p, NULL, 16);
			if (!size)
				return CMD_RET_USAGE;
			efi_clear_bootdev();
		} else {
			/* Image should be already loaded */
			efi_get_image_parameters(&image_addr, &image_size);

			if (image_buf != image_addr) {
				log_err("No UEFI binary known at %s\n",
					argv[1]);
				return CMD_RET_FAILURE;
			}
			size = image_size;
		}
	}

	ret = efi_binary_run(image_buf, size, fdt);

	if (ret != EFI_SUCCESS)
		return CMD_RET_FAILURE;

	return CMD_RET_SUCCESS;
}

U_BOOT_LONGHELP(bootefi,
	"<image address>[:<image size>] [<fdt address>]\n"
	"  - boot EFI payload\n"
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
#ifdef CONFIG_CMD_BOOTEFI_BOOTMGR
	"bootefi bootmgr [fdt address]\n"
	"  - load and boot EFI payload based on BootOrder/BootXXXX variables.\n"
	"\n"
	"    If specified, the device tree located at <fdt address> gets\n"
	"    exposed as EFI configuration table.\n"
#endif
	);

U_BOOT_CMD(
	bootefi, 4, 0, do_bootefi,
	"Boots an EFI payload from memory",
	bootefi_help_text
);
