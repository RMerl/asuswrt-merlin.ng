// SPDX-License-Identifier: GPL-2.0+
/*
 * efi_selftest_watchdog
 *
 * Copyright (c) 2017 Heinrich Schuchardt <xypron.glpk@gmx.de>
 *
 * The 'watchdog timer' unit test checks that the watchdog timer
 * will not cause a system restart during the timeout period after
 * a timer reset.
 *
 * The 'watchdog reboot' unit test checks that the watchdog timer
 * actually reboots the system after a timeout. The test is only
 * executed on explicit request. Use the following commands:
 *
 *	setenv efi_selftest watchdog reboot
 *	bootefi selftest
 */

#include <efi_selftest.h>

/*
 * This is the communication structure for the notification function.
 */
struct notify_context {
	/* Status code returned when resetting watchdog */
	efi_status_t status;
	/* Number of invocations of the notification function */
	unsigned int timer_ticks;
};

static struct efi_event *efi_st_event_notify;
static struct efi_event *efi_st_event_wait;
static struct efi_boot_services *boottime;
static struct notify_context notification_context;
static bool watchdog_reset;

/*
 * Notification function, increments the notification count if parameter
 * context is provided.
 *
 * @event	notified event
 * @context	pointer to the timeout
 */
static void EFIAPI notify(struct efi_event *event, void *context)
{
	struct notify_context *notify_context = context;
	efi_status_t ret = EFI_SUCCESS;

	if (!notify_context)
		return;

	/* Reset watchdog timer to one second */
	ret = boottime->set_watchdog_timer(1, 0, 0, NULL);
	if (ret != EFI_SUCCESS)
		notify_context->status = ret;
	/* Count number of calls */
	notify_context->timer_ticks++;
}

/*
 * Setup unit test.
 *
 * Create two timer events.
 * One with EVT_NOTIFY_SIGNAL, the other with EVT_NOTIFY_WAIT.
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

	notification_context.status = EFI_SUCCESS;
	notification_context.timer_ticks = 0;
	ret = boottime->create_event(EVT_TIMER | EVT_NOTIFY_SIGNAL,
				     TPL_CALLBACK, notify,
				     (void *)&notification_context,
				     &efi_st_event_notify);
	if (ret != EFI_SUCCESS) {
		efi_st_error("could not create event\n");
		return EFI_ST_FAILURE;
	}
	ret = boottime->create_event(EVT_TIMER | EVT_NOTIFY_WAIT,
				     TPL_CALLBACK, notify, NULL,
				     &efi_st_event_wait);
	if (ret != EFI_SUCCESS) {
		efi_st_error("could not create event\n");
		return EFI_ST_FAILURE;
	}
	return EFI_ST_SUCCESS;
}

/*
 * Execute the test resetting the watchdog in a timely manner. No reboot occurs.
 *
 * @handle:	handle of the loaded image
 * @systable:	system table
 * Return:	EFI_ST_SUCCESS for success
 */
static int setup_timer(const efi_handle_t handle,
		       const struct efi_system_table *systable)
{
	watchdog_reset = true;
	return setup(handle, systable);
}

/*
 * Execute the test without resetting the watchdog. A system reboot occurs.
 *
 * @handle:	handle of the loaded image
 * @systable:	system table
 * Return:	EFI_ST_SUCCESS for success
 */
static int setup_reboot(const efi_handle_t handle,
			const struct efi_system_table *systable)
{
	watchdog_reset = false;
	return setup(handle, systable);
}

/*
 * Tear down unit test.
 *
 * Close the events created in setup.
 *
 * Return:	EFI_ST_SUCCESS for success
 */
static int teardown(void)
{
	efi_status_t ret;

	/* Set the watchdog timer to the five minute default value */
	ret = boottime->set_watchdog_timer(300, 0, 0, NULL);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Setting watchdog timer failed\n");
		return EFI_ST_FAILURE;
	}
	if (efi_st_event_notify) {
		ret = boottime->close_event(efi_st_event_notify);
		efi_st_event_notify = NULL;
		if (ret != EFI_SUCCESS) {
			efi_st_error("Could not close event\n");
			return EFI_ST_FAILURE;
		}
	}
	if (efi_st_event_wait) {
		ret = boottime->close_event(efi_st_event_wait);
		efi_st_event_wait = NULL;
		if (ret != EFI_SUCCESS) {
			efi_st_error("Could not close event\n");
			return EFI_ST_FAILURE;
		}
	}
	return EFI_ST_SUCCESS;
}

/*
 * Execute unit test.
 *
 * Run a 600 ms periodic timer that resets the watchdog to one second
 * on every timer tick.
 *
 * Run a 1350 ms single shot timer and check that the 600ms timer has
 * been called 2 times.
 *
 * Return:	EFI_ST_SUCCESS for success
 */
static int execute(void)
{
	size_t index;
	efi_status_t ret;

	/* Set the watchdog timeout to one second */
	ret = boottime->set_watchdog_timer(1, 0, 0, NULL);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Setting watchdog timer failed\n");
		return EFI_ST_FAILURE;
	}
	if (watchdog_reset) {
		/* Set 600 ms timer */
		ret = boottime->set_timer(efi_st_event_notify,
					  EFI_TIMER_PERIODIC, 6000000);
		if (ret != EFI_SUCCESS) {
			efi_st_error("Could not set timer\n");
			return EFI_ST_FAILURE;
		}
	}
	/* Set 1350 ms timer */
	ret = boottime->set_timer(efi_st_event_wait, EFI_TIMER_RELATIVE,
				  13500000);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Could not set timer\n");
		return EFI_ST_FAILURE;
	}

	ret = boottime->wait_for_event(1, &efi_st_event_wait, &index);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Could not wait for event\n");
		return EFI_ST_FAILURE;
	}
	if (notification_context.status != EFI_SUCCESS) {
		efi_st_error("Setting watchdog timer failed\n");
		return EFI_ST_FAILURE;
	}
	if (notification_context.timer_ticks != 2) {
		efi_st_error("The timer was called %u times, expected 2.\n",
			     notification_context.timer_ticks);
		return EFI_ST_FAILURE;
	}
	return EFI_ST_SUCCESS;
}

EFI_UNIT_TEST(watchdog1) = {
	.name = "watchdog timer",
	.phase = EFI_EXECUTE_BEFORE_BOOTTIME_EXIT,
	.setup = setup_timer,
	.execute = execute,
	.teardown = teardown,
};

EFI_UNIT_TEST(watchdog2) = {
	.name = "watchdog reboot",
	.phase = EFI_EXECUTE_BEFORE_BOOTTIME_EXIT,
	.setup = setup_reboot,
	.execute = execute,
	.teardown = teardown,
	.on_request = true,
};
