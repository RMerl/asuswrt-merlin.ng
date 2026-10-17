// SPDX-License-Identifier: GPL-2.0+
/*
 * efi_selftest_memory
 *
 * Copyright (c) 2018 Heinrich Schuchardt <xypron.glpk@gmx.de>
 *
 * This unit test checks the following boottime services:
 * CopyMem, SetMem, CalculateCrc32
 *
 * The memory type used for the device tree is checked.
 */

#include <efi_selftest.h>

static struct efi_boot_services *boottime;

/**
 * setup() - setup unit test
 *
 * @handle:	handle of the loaded image
 * @systable:	system table
 * Return:	EFI_ST_SUCCESS for success
 */
static int setup(const efi_handle_t handle,
		 const struct efi_system_table *systable)
{
	boottime = systable->boottime;

	return EFI_ST_SUCCESS;
}

/*
 * execute() - execute unit test
 *
 * Return:	EFI_ST_SUCCESS for success
 */
static int execute(void)
{
	u8 c1[] = "abcdefghijklmnop";
	u8 c2[] = "abcdefghijklmnop";
	u32 crc32;
	efi_status_t ret;

	ret = boottime->calculate_crc32(c1, 16, &crc32);
	if (ret != EFI_SUCCESS) {
		efi_st_error("CalculateCrc32 failed\n");
		return EFI_ST_FAILURE;
	}
	if (crc32 != 0x943ac093) {
		efi_st_error("CalculateCrc32 returned wrong value\n");
		return EFI_ST_FAILURE;
	}
	boottime->copy_mem(&c1[5], &c1[3], 8);
	if (memcmp(c1, "abcdedefghijknop", 16)) {
		efi_st_error("CopyMem forward copy failed: %s\n", c1);
		return EFI_ST_FAILURE;
	}
	boottime->copy_mem(&c2[3], &c2[5], 8);
	if (memcmp(c2, "abcfghijklmlmnop", 16)) {
		efi_st_error("CopyMem backward copy failed: %s\n", c2);
		return EFI_ST_FAILURE;
	}
	boottime->set_mem(&c1[3], 8, 'x');
	if (memcmp(c1, "abcxxxxxxxxjknop", 16)) {
		efi_st_error("SetMem failed: %s\n", c1);
		return EFI_ST_FAILURE;
	}

	return EFI_ST_SUCCESS;
}

EFI_UNIT_TEST(mem) = {
	.name = "mem",
	.phase = EFI_EXECUTE_BEFORE_BOOTTIME_EXIT,
	.setup = setup,
	.execute = execute,
};
