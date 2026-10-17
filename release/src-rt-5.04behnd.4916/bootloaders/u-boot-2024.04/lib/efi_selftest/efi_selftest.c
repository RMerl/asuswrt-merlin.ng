// SPDX-License-Identifier: GPL-2.0+
/*
 * EFI efi_selftest
 *
 * Copyright (c) 2017 Heinrich Schuchardt <xypron.glpk@gmx.de>
 */

#include <command.h>
#include <efi_selftest.h>
#include <vsprintf.h>

/* Constants for test step bitmap */
#define EFI_ST_SETUP	1
#define EFI_ST_EXECUTE	2
#define EFI_ST_TEARDOWN	4

const struct efi_system_table *st_systable;
const struct efi_boot_services *st_boottime;
static const struct efi_runtime_services *runtime;
static efi_handle_t handle;
static u16 reset_message[] = u"Selftest completed";
static int *setup_status;

/*
 * Exit the boot services.
 *
 * The size of the memory map is determined.
 * Pool memory is allocated to copy the memory map.
 * The memory map is copied and the map key is obtained.
 * The map key is used to exit the boot services.
 */
void efi_st_exit_boot_services(void)
{
	efi_uintn_t map_size = 0;
	efi_uintn_t map_key;
	efi_uintn_t desc_size;
	u32 desc_version;
	efi_status_t ret;
	struct efi_mem_desc *memory_map;

	/* Do not detach devices in ExitBootServices. We need the console. */
	efi_st_keep_devices = true;

	ret = st_boottime->get_memory_map(&map_size, NULL, &map_key, &desc_size,
				       &desc_version);
	if (ret != EFI_BUFFER_TOO_SMALL) {
		efi_st_error(
			"GetMemoryMap did not return EFI_BUFFER_TOO_SMALL\n");
		return;
	}
	/* Allocate extra space for newly allocated memory */
	map_size += sizeof(struct efi_mem_desc);
	ret = st_boottime->allocate_pool(EFI_BOOT_SERVICES_DATA, map_size,
				      (void **)&memory_map);
	if (ret != EFI_SUCCESS) {
		efi_st_error("AllocatePool did not return EFI_SUCCESS\n");
		return;
	}
	ret = st_boottime->get_memory_map(&map_size, memory_map, &map_key,
				       &desc_size, &desc_version);
	if (ret != EFI_SUCCESS) {
		efi_st_error("GetMemoryMap did not return EFI_SUCCESS\n");
		return;
	}
	ret = st_boottime->exit_boot_services(handle, map_key);
	if (ret != EFI_SUCCESS) {
		efi_st_error("ExitBootServices did not return EFI_SUCCESS\n");
		return;
	}
	efi_st_printc(EFI_WHITE, "\nBoot services terminated\n");
}

/*
 * Set up a test.
 *
 * @test	the test to be executed
 * @failures	counter that will be incremented if a failure occurs
 * Return:	EFI_ST_SUCCESS for success
 */
static int setup(struct efi_unit_test *test, unsigned int *failures)
{
	int ret;

	if (!test->setup)
		return EFI_ST_SUCCESS;
	efi_st_printc(EFI_LIGHTBLUE, "\nSetting up '%s'\n", test->name);
	ret = test->setup(handle, st_systable);
	if (ret != EFI_ST_SUCCESS) {
		efi_st_error("Setting up '%s' failed\n", test->name);
		++*failures;
	} else {
		efi_st_printc(EFI_LIGHTGREEN,
			      "Setting up '%s' succeeded\n", test->name);
	}
	return ret;
}

/*
 * Execute a test.
 *
 * @test	the test to be executed
 * @failures	counter that will be incremented if a failure occurs
 * Return:	EFI_ST_SUCCESS for success
 */
static int execute(struct efi_unit_test *test, unsigned int *failures)
{
	int ret;

	if (!test->execute)
		return EFI_ST_SUCCESS;
	efi_st_printc(EFI_LIGHTBLUE, "\nExecuting '%s'\n", test->name);
	ret = test->execute();
	if (ret != EFI_ST_SUCCESS) {
		efi_st_error("Executing '%s' failed\n", test->name);
		++*failures;
	} else {
		efi_st_printc(EFI_LIGHTGREEN,
			      "Executing '%s' succeeded\n", test->name);
	}
	return ret;
}

/*
 * Tear down a test.
 *
 * @test	the test to be torn down
 * @failures	counter that will be incremented if a failure occurs
 * Return:	EFI_ST_SUCCESS for success
 */
static int teardown(struct efi_unit_test *test, unsigned int *failures)
{
	int ret;

	if (!test->teardown)
		return EFI_ST_SUCCESS;
	efi_st_printc(EFI_LIGHTBLUE, "\nTearing down '%s'\n", test->name);
	ret = test->teardown();
	if (ret != EFI_ST_SUCCESS) {
		efi_st_error("Tearing down '%s' failed\n", test->name);
		++*failures;
	} else {
		efi_st_printc(EFI_LIGHTGREEN,
			      "Tearing down '%s' succeeded\n", test->name);
	}
	return ret;
}

/*
 * Check that a test requiring reset exists.
 *
 * @testname:	name of the test
 * Return:	test, or NULL if not found
 */
static bool need_reset(const u16 *testname)
{
	struct efi_unit_test *test;

	for (test = ll_entry_start(struct efi_unit_test, efi_unit_test);
	     test < ll_entry_end(struct efi_unit_test, efi_unit_test); ++test) {
		if (testname && efi_st_strcmp_16_8(testname, test->name))
			continue;
		if (test->phase == EFI_SETUP_BEFORE_BOOTTIME_EXIT ||
		    test->phase == EFI_SETTING_VIRTUAL_ADDRESS_MAP)
			return true;
	}
	return false;
}

/*
 * Check that a test exists.
 *
 * @testname:	name of the test
 * Return:	test, or NULL if not found
 */
static struct efi_unit_test *find_test(const u16 *testname)
{
	struct efi_unit_test *test;

	for (test = ll_entry_start(struct efi_unit_test, efi_unit_test);
	     test < ll_entry_end(struct efi_unit_test, efi_unit_test); ++test) {
		if (!efi_st_strcmp_16_8(testname, test->name))
			return test;
	}
	efi_st_printf("\nTest '%ps' not found\n", testname);
	return NULL;
}

/*
 * List all available tests.
 */
static void list_all_tests(void)
{
	struct efi_unit_test *test;

	/* List all tests */
	efi_st_printf("\nAvailable tests:\n");
	for (test = ll_entry_start(struct efi_unit_test, efi_unit_test);
	     test < ll_entry_end(struct efi_unit_test, efi_unit_test); ++test) {
		efi_st_printf("'%s'%s\n", test->name,
			      test->on_request ? " - on request" : "");
	}
}

/*
 * Execute test steps of one phase.
 *
 * @testname	name of a single selected test or NULL
 * @phase	test phase
 * @steps	steps to execute (mask with bits from EFI_ST_...)
 * failures	returns EFI_ST_SUCCESS if all test steps succeeded
 */
void efi_st_do_tests(const u16 *testname, unsigned int phase,
		     unsigned int steps, unsigned int *failures)
{
	int i = 0;
	struct efi_unit_test *test;

	for (test = ll_entry_start(struct efi_unit_test, efi_unit_test);
	     test < ll_entry_end(struct efi_unit_test, efi_unit_test);
	     ++test, ++i) {
		if (testname ?
		    efi_st_strcmp_16_8(testname, test->name) : test->on_request)
			continue;
		if (test->phase != phase)
			continue;
		if (steps & EFI_ST_SETUP)
			setup_status[i] = setup(test, failures);
		if (steps & EFI_ST_EXECUTE && setup_status[i] == EFI_ST_SUCCESS)
			execute(test, failures);
		if (steps & EFI_ST_TEARDOWN)
			teardown(test, failures);
	}
}

/*
 * Execute selftest of the EFI API
 *
 * This is the main entry point of the EFI selftest application.
 *
 * All tests use a driver model and are run in three phases:
 * setup, execute, teardown.
 *
 * A test may be setup and executed at st_boottime,
 * it may be setup at st_boottime and executed at runtime,
 * or it may be setup and executed at runtime.
 *
 * After executing all tests the system is reset.
 *
 * @image_handle:	handle of the loaded EFI image
 * @systab:		EFI system table
 */
efi_status_t EFIAPI efi_selftest(efi_handle_t image_handle,
				 struct efi_system_table *systab)
{
	unsigned int failures = 0;
	const u16 *testname = NULL;
	struct efi_loaded_image *loaded_image;
	efi_status_t ret;

	st_systable = systab;
	st_boottime = st_systable->boottime;
	runtime = st_systable->runtime;
	handle = image_handle;
	con_out = st_systable->con_out;
	con_in = st_systable->con_in;

	ret = st_boottime->handle_protocol(image_handle, &efi_guid_loaded_image,
					(void **)&loaded_image);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Cannot open loaded image protocol\n");
		return ret;
	}

	if (loaded_image->load_options)
		testname = (u16 *)loaded_image->load_options;

	if (testname) {
		if (!efi_st_strcmp_16_8(testname, "list") ||
		    !find_test(testname)) {
			list_all_tests();
			/*
			 * TODO:
			 * Once the Exit st_boottime service is correctly
			 * implemented we should call
			 *   st_boottime->exit(image_handle, EFI_SUCCESS, 0, NULL);
			 * here, cf.
			 * https://lists.denx.de/pipermail/u-boot/2017-October/308720.html
			 */
			return EFI_SUCCESS;
		}
	}

	efi_st_printc(EFI_WHITE, "\nTesting EFI API implementation\n");

	if (testname)
		efi_st_printc(EFI_WHITE, "\nSelected test: '%ps'\n", testname);
	else
		efi_st_printc(EFI_WHITE, "\nNumber of tests to execute: %u\n",
			      ll_entry_count(struct efi_unit_test,
					     efi_unit_test));

	/* Allocate buffer for setup results */
	ret = st_boottime->allocate_pool(EFI_RUNTIME_SERVICES_DATA, sizeof(int) *
				      ll_entry_count(struct efi_unit_test,
						     efi_unit_test),
				      (void **)&setup_status);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Allocate pool failed\n");
		return ret;
	}

	/* Execute st_boottime tests */
	efi_st_do_tests(testname, EFI_EXECUTE_BEFORE_BOOTTIME_EXIT,
			EFI_ST_SETUP | EFI_ST_EXECUTE | EFI_ST_TEARDOWN,
			&failures);

	if (!need_reset(testname)) {
		if (failures)
			ret = EFI_PROTOCOL_ERROR;

		/* Give feedback */
		efi_st_printc(EFI_WHITE, "\nSummary: %u failures\n\n",
			      failures);
		return ret;
	}

	/* Execute mixed tests */
	efi_st_do_tests(testname, EFI_SETUP_BEFORE_BOOTTIME_EXIT,
			EFI_ST_SETUP, &failures);
	efi_st_do_tests(testname, EFI_SETTING_VIRTUAL_ADDRESS_MAP,
			EFI_ST_SETUP, &failures);

	efi_st_exit_boot_services();

	efi_st_do_tests(testname, EFI_SETUP_BEFORE_BOOTTIME_EXIT,
			EFI_ST_EXECUTE | EFI_ST_TEARDOWN, &failures);
	/* Execute test setting the virtual address map */
	efi_st_do_tests(testname, EFI_SETTING_VIRTUAL_ADDRESS_MAP,
			EFI_ST_EXECUTE | EFI_ST_TEARDOWN,
			&failures);

	/* Give feedback */
	efi_st_printc(EFI_WHITE, "\nSummary: %u failures\n\n", failures);

	/* Reset system */
	efi_st_printf("Preparing for reset. Press any key...\n");
	efi_st_get_key();

	if (IS_ENABLED(CONFIG_EFI_HAVE_RUNTIME_RESET)) {
		runtime->reset_system(EFI_RESET_WARM, EFI_NOT_READY,
				      sizeof(reset_message), reset_message);
	} else {
		efi_restore_gd();
		do_reset(NULL, 0, 0, NULL);
	}

	efi_st_printf("\n");
	efi_st_error("Reset failed\n");

	return EFI_UNSUPPORTED;
}
