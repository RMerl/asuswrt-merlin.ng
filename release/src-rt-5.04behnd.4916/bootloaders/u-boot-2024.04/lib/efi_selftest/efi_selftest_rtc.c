// SPDX-License-Identifier: GPL-2.0+
/*
 * efi_selftest_rtc
 *
 * Copyright (c) 2018 Heinrich Schuchardt <xypron.glpk@gmx.de>
 *
 * Test the real time clock runtime services.
 */

#include <efi_selftest.h>

#define EFI_ST_NO_RTC "Could not read real time clock\n"
#define EFI_ST_NO_RTC_SET "Could not set real time clock\n"

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
 * Read and display current time.
 * Set a new value and read it back.
 * Set the real time clock back the current time.
 *
 * Return:	EFI_ST_SUCCESS for success
 */
static int execute(void)
{
	efi_status_t ret;
	struct efi_time tm_old;
#ifdef CONFIG_EFI_SET_TIME
	struct efi_time tm, tm_new = {
		.year = 2017,
		.month = 5,
		.day = 19,
		.hour = 13,
		.minute = 47,
		.second = 53,
	};
#endif

	/* Display current time */
	ret = runtime->get_time(&tm_old, NULL);
	if (ret != EFI_SUCCESS) {
		efi_st_error(EFI_ST_NO_RTC);
		return EFI_ST_FAILURE;
	}
	efi_st_printf("Time according to real time clock: "
		      "%.4u-%.2u-%.2u %.2u:%.2u:%.2u\n",
		      tm_old.year, tm_old.month, tm_old.day,
		      tm_old.hour, tm_old.minute, tm_old.second);
#ifdef CONFIG_EFI_SET_TIME
	ret = runtime->set_time(&tm_new);
	if (ret != EFI_SUCCESS) {
		efi_st_error(EFI_ST_NO_RTC_SET);
		return EFI_ST_FAILURE;
	}
	ret = runtime->get_time(&tm, NULL);
	if (ret != EFI_SUCCESS) {
		efi_st_error(EFI_ST_NO_RTC);
		return EFI_ST_FAILURE;
	}
	if (tm.year != tm_new.year ||
	    tm.month != tm_new.month ||
	    tm.day != tm_new.day ||
	    tm.hour !=  tm_new.hour ||
	    tm.minute != tm_new.minute ||
	    tm.second < tm_new.second ||
	    tm.second > tm_new.second + 2) {
		efi_st_error(EFI_ST_NO_RTC_SET);
		return EFI_ST_FAILURE;
	}
	/* Set time back to old value */
	ret = runtime->set_time(&tm_old);
	if (ret != EFI_SUCCESS) {
		efi_st_error(EFI_ST_NO_RTC_SET);
		return EFI_ST_FAILURE;
	}
#endif

	return EFI_ST_SUCCESS;
}

EFI_UNIT_TEST(rtc) = {
	.name = "real time clock",
	.phase = EFI_EXECUTE_BEFORE_BOOTTIME_EXIT,
	.setup = setup,
	.execute = execute,
};
