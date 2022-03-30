// SPDX-License-Identifier: GPL-2.0+
/*
 * efi_selftest_event_groups
 *
 * Copyright (c) 2018 Heinrich Schuchardt <xypron.glpk@gmx.de>
 *
 * This test checks the notification of group events and the
 * following services:
 * CreateEventEx, CloseEvent, SignalEvent, CheckEvent.
 */

#include <efi_selftest.h>

#define GROUP_SIZE 16

static struct efi_boot_services *boottime;
static efi_guid_t event_group =
	EFI_GUID(0x2335905b, 0xc3b9, 0x4221, 0xa3, 0x71,
		 0x0e, 0x5b, 0x45, 0xc0, 0x56, 0x91);

/*
 * Notification function, increments the notification count if parameter
 * context is provided.
 *
 * @event	notified event
 * @context	pointer to the notification count
 */
static void EFIAPI notify(struct efi_event *event, void *context)
{
	unsigned int *count = context;

	if (count)
		++*count;
}

/*
 * Setup unit test.
 *
 * @handle:	handle of the loaded image
 * @systable:	system table
 * @return:	EFI_ST_SUCCESS for success
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
 * Create multiple events in an event group. Signal each event once and check
 * that all events are notified once in each round.
 *
 * @return:	EFI_ST_SUCCESS for success
 */
static int execute(void)
{
	unsigned int counter[GROUP_SIZE] = {0};
	struct efi_event *events[GROUP_SIZE];
	size_t i, j;
	efi_status_t ret;

	for (i = 0; i < GROUP_SIZE; ++i) {
		ret = boottime->create_event_ex(0, TPL_NOTIFY,
						notify, (void *)&counter[i],
						&event_group, &events[i]);
		if (ret != EFI_SUCCESS) {
			efi_st_error("Failed to create event\n");
			return EFI_ST_FAILURE;
		}
	}

	for (i = 0; i < GROUP_SIZE; ++i) {
		ret = boottime->signal_event(events[i]);
		if (ret != EFI_SUCCESS) {
			efi_st_error("Failed to signal event\n");
			return EFI_ST_FAILURE;
		}
		for (j = 0; j < GROUP_SIZE; ++j) {
			if (counter[j] != 2 * i + 1) {
				efi_st_printf("i %u, j %u, count %u\n",
					      (unsigned int)i, (unsigned int)j,
					      (unsigned int)counter[j]);
				efi_st_error("Notification function was not called\n");
				return EFI_ST_FAILURE;
			}
			/* Clear signaled state */
			ret = boottime->check_event(events[j]);
			if (ret != EFI_SUCCESS) {
				efi_st_error("Event was not signaled\n");
				return EFI_ST_FAILURE;
			}
			if (counter[j] != 2 * i + 1) {
				efi_st_printf("i %u, j %u, count %u\n",
					      (unsigned int)i, (unsigned int)j,
					      (unsigned int)counter[j]);
				efi_st_error(
					"Notification function was called\n");
				return EFI_ST_FAILURE;
			}
			/* Call notification function  */
			ret = boottime->check_event(events[j]);
			if (ret != EFI_NOT_READY) {
				efi_st_error(
					"Signaled state not cleared\n");
				return EFI_ST_FAILURE;
			}
			if (counter[j] != 2 * i + 2) {
				efi_st_printf("i %u, j %u, count %u\n",
					      (unsigned int)i, (unsigned int)j,
					      (unsigned int)counter[j]);
				efi_st_error(
					"Notification function not called\n");
				return EFI_ST_FAILURE;
			}
		}
	}

	for (i = 0; i < GROUP_SIZE; ++i) {
		ret = boottime->close_event(events[i]);
		if (ret != EFI_SUCCESS) {
			efi_st_error("Failed to close event\n");
			return EFI_ST_FAILURE;
		}
	}

	return EFI_ST_SUCCESS;
}

EFI_UNIT_TEST(eventgoups) = {
	.name = "event groups",
	.phase = EFI_EXECUTE_BEFORE_BOOTTIME_EXIT,
	.setup = setup,
	.execute = execute,
};
