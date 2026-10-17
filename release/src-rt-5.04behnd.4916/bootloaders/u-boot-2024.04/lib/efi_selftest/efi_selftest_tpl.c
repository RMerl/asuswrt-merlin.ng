// SPDX-License-Identifier: GPL-2.0+
/*
 * efi_selftest_tpl
 *
 * Copyright (c) 2017 Heinrich Schuchardt <xypron.glpk@gmx.de>
 *
 * This unit test uses timer events to check the handling of
 * task priority levels.
 */

#include <efi_selftest.h>

static struct efi_event *efi_st_event_notify;
static struct efi_event *efi_st_event_wait;
static unsigned int notification_count;
static struct efi_boot_services *boottime;

/*
 * Notification function, increments the notification count.
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
 * Return:	EFI_ST_SUCCESS for success
 */
static int setup(const efi_handle_t handle,
		 const struct efi_system_table *systable)
{
	efi_status_t ret;

	boottime = systable->boottime;

	ret = boottime->create_event(EVT_TIMER | EVT_NOTIFY_SIGNAL,
				     TPL_CALLBACK, notify,
				     (void *)&notification_count,
				     &efi_st_event_notify);
	if (ret != EFI_SUCCESS) {
		efi_st_error("could not create event\n");
		return EFI_ST_FAILURE;
	}
	ret = boottime->create_event(EVT_TIMER | EVT_NOTIFY_WAIT,
				     TPL_NOTIFY, notify, NULL,
				     &efi_st_event_wait);
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
 * Return:	EFI_ST_SUCCESS for success
 */
static int teardown(void)
{
	efi_status_t ret;

	if (efi_st_event_notify) {
		ret = boottime->close_event(efi_st_event_notify);
		efi_st_event_notify = NULL;
		if (ret != EFI_SUCCESS) {
			efi_st_error("could not close event\n");
			return EFI_ST_FAILURE;
		}
	}
	if (efi_st_event_wait) {
		ret = boottime->close_event(efi_st_event_wait);
		efi_st_event_wait = NULL;
		if (ret != EFI_SUCCESS) {
			efi_st_error("could not close event\n");
			return EFI_ST_FAILURE;
		}
	}
	boottime->restore_tpl(TPL_APPLICATION);
	return EFI_ST_SUCCESS;
}

/*
 * Execute unit test.
 *
 * Run a 10 ms periodic timer and check that it is called 10 times
 * while waiting for 100 ms single shot timer.
 *
 * Raise the TPL level to the level of the 10 ms timer and observe
 * that the notification function is not called again.
 *
 * Lower the TPL level and check that the queued notification
 * function is called.
 *
 * Return:	EFI_ST_SUCCESS for success
 */
static int execute(void)
{
	efi_uintn_t index;
	efi_status_t ret;
	efi_uintn_t old_tpl;

	/* Set 10 ms timer */
	notification_count = 0;
	ret = boottime->set_timer(efi_st_event_notify, EFI_TIMER_PERIODIC,
				  100000);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Could not set timer\n");
		return EFI_ST_FAILURE;
	}
	/* Set 100 ms timer */
	ret = boottime->set_timer(efi_st_event_wait, EFI_TIMER_RELATIVE,
				  1000000);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Could not set timer\n");
		return EFI_ST_FAILURE;
	}
	index = 5;
	ret = boottime->wait_for_event(1, &efi_st_event_wait, &index);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Could not wait for event\n");
		return EFI_ST_FAILURE;
	}
	ret = boottime->check_event(efi_st_event_wait);
	if (ret != EFI_NOT_READY) {
		efi_st_error("Signaled state was not cleared.\n");
		efi_st_printf("ret = %u\n", (unsigned int)ret);
		return EFI_ST_FAILURE;
	}
	if (index != 0) {
		efi_st_error("WaitForEvent returned wrong index\n");
		return EFI_ST_FAILURE;
	}
	if (notification_count < 8 || notification_count > 12) {
		efi_st_printf(
		    "Notification count with TPL level TPL_APPLICATION: %u\n",
		    notification_count);
		efi_st_error("Incorrect timing of events\n");
		return EFI_ST_FAILURE;
	}
	ret = boottime->set_timer(efi_st_event_notify, EFI_TIMER_STOP, 0);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Could not cancel timer\n");
		return EFI_ST_FAILURE;
	}
	/* Raise TPL level */
	old_tpl = boottime->raise_tpl(TPL_CALLBACK);
	if (old_tpl != TPL_APPLICATION) {
		efi_st_error("Initial TPL level was not TPL_APPLICATION");
		return EFI_ST_FAILURE;
	}
	/* Set 10 ms timer */
	notification_count = 0;
	ret = boottime->set_timer(efi_st_event_notify, EFI_TIMER_PERIODIC,
				  100000);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Could not set timer\n");
		return EFI_ST_FAILURE;
	}
	/* Set 100 ms timer */
	ret = boottime->set_timer(efi_st_event_wait, EFI_TIMER_RELATIVE,
				  1000000);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Could not set timer\n");
		return EFI_ST_FAILURE;
	}
	do {
		ret = boottime->check_event(efi_st_event_wait);
	} while (ret == EFI_NOT_READY);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Could not check event\n");
		return EFI_ST_FAILURE;
	}
	if (notification_count != 0) {
		efi_st_printf(
			"Notification count with TPL level TPL_CALLBACK: %u\n",
			notification_count);
		efi_st_error("Suppressed timer fired\n");
		return EFI_ST_FAILURE;
	}
	/* Set 1 ms timer */
	ret = boottime->set_timer(efi_st_event_wait, EFI_TIMER_RELATIVE, 1000);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Could not set timer\n");
		return EFI_ST_FAILURE;
	}
	/* Restore the old TPL level */
	boottime->restore_tpl(TPL_APPLICATION);
	ret = boottime->wait_for_event(1, &efi_st_event_wait, &index);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Could not wait for event\n");
		return EFI_ST_FAILURE;
	}
	if (notification_count < 1) {
		efi_st_printf(
		    "Notification count with TPL level TPL_APPLICATION: %u\n",
		    notification_count);
		efi_st_error("Queued timer event did not fire\n");
		return EFI_ST_FAILURE;
	}
	ret = boottime->set_timer(efi_st_event_wait, EFI_TIMER_STOP, 0);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Could not cancel timer\n");
		return EFI_ST_FAILURE;
	}

	return EFI_ST_SUCCESS;
}

EFI_UNIT_TEST(tpl) = {
	.name = "task priority levels",
	.phase = EFI_EXECUTE_BEFORE_BOOTTIME_EXIT,
	.setup = setup,
	.execute = execute,
	.teardown = teardown,
};
