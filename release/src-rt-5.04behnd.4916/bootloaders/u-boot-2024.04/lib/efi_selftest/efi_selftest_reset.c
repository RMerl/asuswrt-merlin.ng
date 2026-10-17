// SPDX-License-Identifier: GPL-2.0+
/*
 * efi_selftest_reset
 *
 * Copyright (c) 2020 Heinrich Schuchardt <xypron.glpk@gmx.de>
 *
 * This test checks the following service at boot time or runtime:
 * ResetSystem()
 */

#include <efi_selftest.h>

static struct efi_runtime_services *runtime;

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
	runtime = systable->runtime;
	return EFI_ST_SUCCESS;
}

/*
 * Execute unit test.
 *
 * Return:	EFI_ST_SUCCESS for success
 */
static int execute(void)
{
	u16 reset_data[] = u"Reset by selftest";

	runtime->reset_system(EFI_RESET_COLD, EFI_SUCCESS,
			      sizeof(reset_data), reset_data);
	efi_st_error("Reset failed.\n");
	return EFI_ST_FAILURE;
}

EFI_UNIT_TEST(reset) = {
	.name = "reset system",
	.phase = EFI_EXECUTE_BEFORE_BOOTTIME_EXIT,
	.setup = setup,
	.execute = execute,
	.on_request = true,
};

EFI_UNIT_TEST(resetrt) = {
	.name = "reset system runtime",
	.phase = EFI_SETUP_BEFORE_BOOTTIME_EXIT,
	.setup = setup,
	.execute = execute,
	.on_request = true,
};
