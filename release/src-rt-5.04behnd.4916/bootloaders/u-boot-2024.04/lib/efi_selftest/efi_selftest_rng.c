// SPDX-License-Identifier: GPL-2.0+
/*
 * efi_selftest_rng
 *
 * Copyright (c) 2019 Heinrich Schuchardt <xypron.glpk@gmx.de>
 *
 * Test the random number generator service.
 */

#include <efi_selftest.h>
#include <efi_rng.h>

#define RNG_LEN 9

static struct efi_boot_services *boottime;
static efi_guid_t efi_rng_guid = EFI_RNG_PROTOCOL_GUID;

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
	boottime = systable->boottime;
	return EFI_ST_SUCCESS;
}

/*
 * Execute unit test.
 *
 * Retrieve available RNG algorithms.
 * Retrieve two random values and compare them.
 *
 * Return:	EFI_ST_SUCCESS for success
 */
static int execute(void)
{
	efi_status_t ret;
	efi_uintn_t size;
	struct efi_rng_protocol *rng;
	efi_guid_t *algo_list;
	u8 rnd1[RNG_LEN] __aligned(4), rnd2[RNG_LEN] __aligned(4);
	int r;

	/* Get random number generator protocol */
	ret = boottime->locate_protocol(&efi_rng_guid, NULL, (void **)&rng);
	if (ret != EFI_SUCCESS) {
		efi_st_error(
			"Random number generator protocol not available\n");
		return EFI_ST_FAILURE;
	}

	ret = rng->get_info(rng, &size, NULL);
	if (ret != EFI_BUFFER_TOO_SMALL) {
		efi_st_error("Could not retrieve alorithm list size\n");
		return EFI_ST_FAILURE;
	}
	if (size < sizeof(efi_guid_t)) {
		efi_st_error("Empty alorithm list\n");
		return EFI_ST_FAILURE;
	}

	ret = boottime->allocate_pool(EFI_LOADER_DATA, size,
				      (void **)&algo_list);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Could not allocate pool memory\n");
		return EFI_ST_FAILURE;
	}

	ret = rng->get_info(rng, &size, algo_list);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Could not get info\n");
		return EFI_ST_FAILURE;
	}
	if (size < sizeof(efi_guid_t)) {
		efi_st_error("Empty alorithm list\n");
		return EFI_ST_FAILURE;
	}

	memset(rnd1, 0, RNG_LEN);
	memset(rnd2, 0, RNG_LEN);

	ret = rng->get_rng(rng, NULL, RNG_LEN - 1, &rnd1[1]);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Could not get random value\n");
		return EFI_ST_FAILURE;
	}
	ret = rng->get_rng(rng, algo_list, RNG_LEN - 1, &rnd2[1]);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Could not get random value\n");
		return EFI_ST_FAILURE;
	}
	r = memcmp(rnd1, rnd2, RNG_LEN);
	if (!r) {
		efi_st_error("Two equal consecutive random numbers\n");
		return EFI_ST_FAILURE;
	}

	ret = boottime->free_pool(algo_list);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Could not free pool memory\n");
		return EFI_ST_FAILURE;
	}

	return EFI_ST_SUCCESS;
}

EFI_UNIT_TEST(rng) = {
	.name = "random number generator",
	.phase = EFI_EXECUTE_BEFORE_BOOTTIME_EXIT,
	.setup = setup,
	.execute = execute,
};
