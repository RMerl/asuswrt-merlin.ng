// SPDX-License-Identifier: GPL-2.0+
/*
 * efi_selftest_gop
 *
 * Copyright (c) 2017 Heinrich Schuchardt <xypron.glpk@gmx.de>
 *
 * Test the graphical output protocol.
 */

#include <efi_selftest.h>

static struct efi_boot_services *boottime;
static efi_guid_t efi_gop_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
static struct efi_gop *gop;

/*
 * Setup unit test.
 *
 * @handle:	handle of the loaded image
 * @systable:	system table
 * Return:	EFI_ST_SUCCESS for success
 */
static int setup(const efi_handle_t handle,
		 const struct efi_system_table *systable)
{
	efi_status_t ret;

	boottime = systable->boottime;

	ret = boottime->locate_protocol(&efi_gop_guid, NULL, (void **)&gop);
	if (ret != EFI_SUCCESS) {
		gop = NULL;
		efi_st_printf("Graphical output protocol is not available.\n");
	}

	return EFI_ST_SUCCESS;
}

/*
 * Tear down unit test.
 *
 * Return:	EFI_ST_SUCCESS for success
 */
static int teardown(void)
{
	return EFI_ST_SUCCESS;
}

/*
 * Execute unit test.
 *
 * Return:	EFI_ST_SUCCESS for success
 */
static int execute(void)
{
	efi_status_t ret;
	u32 i, max_mode;
	efi_uintn_t size;
	struct efi_gop_mode_info *info;

	if (!gop)
		return EFI_ST_SUCCESS;

	if (!gop->mode) {
		efi_st_error("EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE missing\n");
		return EFI_ST_FAILURE;
	}
	max_mode = gop->mode->max_mode;
	if (!max_mode) {
		efi_st_error("No graphical mode available\n");
		return EFI_ST_FAILURE;
	}
	efi_st_printf("Number of available modes: %u\n", max_mode);

	for (i = 0; i < max_mode; ++i) {
		ret = gop->query_mode(gop, i, &size, &info);
		if (ret != EFI_SUCCESS) {
			efi_st_printf("Could not query mode %u\n", i);
			return EFI_ST_FAILURE;
		}
		efi_st_printf("Mode %u: %u x %u\n",
			      i, info->width, info->height);
		ret = boottime->free_pool(info);
		if (ret != EFI_SUCCESS) {
			efi_st_printf("FreePool failed");
			return EFI_ST_FAILURE;
		}
	}

	return EFI_ST_SUCCESS;
}

EFI_UNIT_TEST(gop) = {
	.name = "graphical output",
	.phase = EFI_EXECUTE_BEFORE_BOOTTIME_EXIT,
	.setup = setup,
	.execute = execute,
	.teardown = teardown,
};
