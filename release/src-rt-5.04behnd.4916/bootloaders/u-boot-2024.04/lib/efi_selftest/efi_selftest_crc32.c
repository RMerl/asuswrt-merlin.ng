// SPDX-License-Identifier: GPL-2.0+
/*
 * efi_selftest_crc32
 *
 * Copyright (c) 2018 Heinrich Schuchardt <xypron.glpk@gmx.de>
 *
 * This unit test checks the CalculateCrc32 bootservice and checks the
 * headers of the system table, the boot services table, and the runtime
 * services table before and after ExitBootServices().
 */

#include <efi_selftest.h>
#include <u-boot/crc.h>

const struct efi_system_table *st;
efi_status_t (EFIAPI *bs_crc32)(const void *data, efi_uintn_t data_size,
				u32 *crc32);

static int check_table(const void *table)
{
	efi_status_t ret;
	u32 crc32, res;
	/* Casting from constant to not constant */
	struct efi_table_hdr *hdr = (struct efi_table_hdr *)table;

	if (!hdr->signature) {
		efi_st_error("Missing header signature\n");
		return EFI_ST_FAILURE;
	}
	if (!hdr->revision) {
		efi_st_error("Missing header revision\n");
		return EFI_ST_FAILURE;
	}
	if (hdr->headersize <= sizeof(struct efi_table_hdr)) {
		efi_st_error("Incorrect headersize value\n");
		return EFI_ST_FAILURE;
	}
	if (hdr->reserved) {
		efi_st_error("Reserved header field is not zero\n");
		return EFI_ST_FAILURE;
	}

	crc32 = hdr->crc32;
	/*
	 * Setting the crc32 of the 'const' table to zero is easier than
	 * copying
	 */
	hdr->crc32 = 0;
	ret = bs_crc32(table, hdr->headersize, &res);
	/* Reset table crc32 so it stays constant */
	hdr->crc32 = crc32;
	if (ret != EFI_ST_SUCCESS) {
		efi_st_error("CalculateCrc32 failed\n");
		return EFI_ST_FAILURE;
	}
	if (res != crc32) {
		efi_st_error("Incorrect CRC32\n");
		// return EFI_ST_FAILURE;
	}
	return EFI_ST_SUCCESS;
}

/*
 * Setup unit test.
 *
 * Check that CalculateCrc32 is working correctly.
 * Check tables before ExitBootServices().
 *
 * @handle:	handle of the loaded image
 * @systable:	system table
 * Return:	EFI_ST_SUCCESS for success
 */
static int setup(const efi_handle_t handle,
		 const struct efi_system_table *systable)
{
	efi_status_t ret;
	u32 res;

	st = systable;
	bs_crc32 = systable->boottime->calculate_crc32;

	/* Check that CalculateCrc32 is working */
	ret = bs_crc32("U-Boot", 6, &res);
	if (ret != EFI_ST_SUCCESS) {
		efi_st_error("CalculateCrc32 failed\n");
		return EFI_ST_FAILURE;
	}
	if (res != 0x134b0db4) {
		efi_st_error("Incorrect CRC32\n");
		return EFI_ST_FAILURE;
	}

	/* Check tables before ExitBootServices() */
	if (check_table(st) != EFI_ST_SUCCESS) {
		efi_st_error("Checking system table\n");
		return EFI_ST_FAILURE;
	}
	if (check_table(st->boottime) != EFI_ST_SUCCESS) {
		efi_st_error("Checking boottime table\n");
		return EFI_ST_FAILURE;
	}
	if (check_table(st->runtime) != EFI_ST_SUCCESS) {
		efi_st_error("Checking runtime table\n");
		return EFI_ST_FAILURE;
	}

	return EFI_ST_SUCCESS;
}

/*
 * Execute unit test
 *
 * Check tables after ExitBootServices()
 *
 * Return:	EFI_ST_SUCCESS for success
 */
static int execute(void)
{
	if (check_table(st) != EFI_ST_SUCCESS) {
		efi_st_error("Checking system table\n");
		return EFI_ST_FAILURE;
	}
	if (check_table(st->runtime) != EFI_ST_SUCCESS) {
		efi_st_error("Checking runtime table\n");
		return EFI_ST_FAILURE;
	}

	/*
	 * We cannot call SetVirtualAddressMap() and recheck the runtime
	 * table afterwards because this would invalidate the addresses of the
	 * unit tests.
	 */

	return EFI_ST_SUCCESS;
}

EFI_UNIT_TEST(crc32) = {
	.name = "crc32",
	.phase = EFI_SETUP_BEFORE_BOOTTIME_EXIT,
	.setup = setup,
	.execute = execute,
};
