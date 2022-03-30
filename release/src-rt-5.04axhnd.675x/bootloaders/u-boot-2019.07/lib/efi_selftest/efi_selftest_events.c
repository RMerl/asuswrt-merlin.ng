// SPDX-License-Identifier: GPL-2.0+
/*
 * efi_selftest_events
 *
 * Copyright (c) 2017 Heinrich Schuchardt <xypron.glpk@gmx.de>
 *
 * This unit test uses timer events to check the implementation
 * of the following boottime services:
 * CreateEvent, CloseEvent, WaitForEvent, CheckEvent, SetTimer.
 */

#include <efi_selftest.h>

static struct efi_event *event_notify;
static struct efi_event *event_wait;
static unsigned int timer_ticks;
static struct efi_boot_services *boottime;

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
 * Create two timer events.
 * One with EVT_NOTIFY_SIGNAL, the other with EVT_NOTIFY_WAIT.
 *
 * @handle:	handle of the loaded image
 * @systable:	system table
 * @return:	EFI_ST_SUCCESS for success
 */
static int setup(const efi_handle_t handle,
		 const struct efi_system_table *systable)
{
	efi_status_t ret;

	boottime = systable->boottime;

	ret = boottime->create_event(EVT_TIMER | EVT_NOTIFY_SIGNAL,
				     TPL_CALLBACK, notify, (void *)&timer_ticks,
				     &event_notify);
	if (ret != EFI_SUCCESS) {
		efi_st_error("could not create event\n");
		return EFI_ST_FAILURE;
	}
	ret = boottime->create_event(EVT_TIMER | EVT_NOTIFY_WAIT,
				     TPL_CALLBACK, notify, NULL, &event_wait);
	if (ret != EFI_SUCCESS) {
		efi_st_error("could not create event\n");
		return EFI_ST_FAILURE;
	}
	return EFI_ST_SUCCESS;
}

/*
 * Tear down unit test.
 *
 * Close the events created in setup.
 *
 * @return:	EFI_ST_SUCCESS for success
 */
static int teardown(void)
{
	efi_status_t ret;

	if (event_notify) {
		ret = boottime->close_event(event_notify);
		event_notify = NULL;
		if (ret != EFI_SUCCESS) {
			efi_st_error("could not close event\n");
			return EFI_ST_FAILURE;
		}
	}
	if (event_wait) {
		ret = boottime->close_event(event_wait);
		event_wait = NULL;
		if (ret != EFI_SUCCESS) {
			efi_st_error("could not close event\n");
			return EFI_ST_FAILURE;
		}
	}
	return EFI_ST_SUCCESS;
}

/*
 * Execute unit test.
 *
 * Run a 10 ms periodic timer and check that it is called 10 times
 * while waiting for 100 ms single shot timer.
 *
 * Run a 100 ms single shot timer and check that it is called once
 * while waiting for 100 ms periodic timer for two periods.
 *
 * @return:	EFI_ST_SUCCESS for success
 */
static int execute(void)
{
	efi_uintn_t index;
	efi_status_t ret;

	/* Set 10 ms timer */
	timer_ticks = 0;
	ret = boottime->set_timer(event_notify, EFI_TIMER_PERIODIC, 100000);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Could not set timer\n");
		return EFI_ST_FAILURE;
	}
	/* Set 100 ms timer */
	ret = boottime->set_timer(event_wait, EFI_TIMER_RELATIVE, 1000000);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Could not set timer\n");
		return EFI_ST_FAILURE;
	}

	/* Set some arbitrary non-zero value to make change detectable. */
	index = 5;
	ret = boottime->wait_for_event(1, &event_wait, &index);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Could not wait for event\n");
		return EFI_ST_FAILURE;
	}
	ret = boottime->check_event(event_wait);
	if (ret != EFI_NOT_READY) {
		efi_st_error("Signaled state was not cleared.\n");
		efi_st_printf("ret = %u\n", (unsigned int)ret);
		return EFI_ST_FAILURE;
	}
	if (index != 0) {
		efi_st_error("WaitForEvent returned wrong index\n");
		return EFI_ST_FAILURE;
	}
	if (timer_ticks < 8 || timer_ticks > 12) {
		efi_st_printf("Notification count periodic: %u\n", timer_ticks);
		efi_st_error("Incorrect timing of events\n");
		return EFI_ST_FAILURE;
	}
	ret = boottime->set_timer(event_notify, EFI_TIMER_STOP, 0);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Could not cancel timer\n");
		return EFI_ST_FAILURE;
	}
	/* Set 10 ms timer */
	timer_ticks = 0;
	ret = boottime->set_timer(event_notify, EFI_TIMER_RELATIVE, 100000);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Could not set timer\n");
		return EFI_ST_FAILURE;
	}
	/* Set 100 ms timer */
	ret = boottime->set_timer(event_wait, EFI_TIMER_PERIODIC, 1000000);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Could not set timer\n");
		return EFI_ST_FAILURE;
	}
	ret = boottime->wait_for_event(1, &event_wait, &index);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Could not wait for event\n");
		return EFI_ST_FAILURE;
	}
	if (timer_ticks != 1) {
		efi_st_printf("Notification count single shot: %u\n",
			      timer_ticks);
		efi_st_error("Single shot timer failed\n");
		return EFI_ST_FAILURE;
	}
	ret = boottime->wait_for_event(1, &event_wait, &index);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Could not wait for event\n");
		return EFI_ST_FAILURE;
	}
	if (timer_ticks != 1) {
		efi_st_printf("Notification count stopped timer: %u\n",
			      timer_ticks);
		efi_st_error("Stopped timer fired\n");
		return EFI_ST_FAILURE;
	}
	ret = boottime->set_timer(event_wait, EFI_TIMER_STOP, 0);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Could not cancel timer\n");
		return EFI_ST_FAILURE;
	}

	return EFI_ST_SUCCESS;
}

EFI_UNIT_TEST(events) = {
	.name = "event services",
	.phase = EFI_EXECUTE_BEFORE_BOOTTIME_EXIT,
	.setup = setup,
	.execute = execute,
	.teardown = teardown,
};
