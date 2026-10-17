// SPDX-License-Identifier: GPL-2.0+
/*
 *  EFI watchdog
 *
 *  Copyright (c) 2017 Heinrich Schuchardt
 */

#include <efi_loader.h>

/* Conversion factor from seconds to multiples of 100ns */
#define EFI_SECONDS_TO_100NS 10000000ULL

static struct efi_event *watchdog_timer_event;

/**
 * efi_watchdog_timer_notify() - resets system upon watchdog event
 *
 * Reset the system when the watchdog event is notified.
 *
 * @event:	the watchdog event
 * @context:	not used
 */
static void EFIAPI efi_watchdog_timer_notify(struct efi_event *event,
					     void *context)
{
	EFI_ENTRY("%p, %p", event, context);

	printf("\nEFI: Watchdog timeout\n");
	do_reset(NULL, 0, 0, NULL);

	EFI_EXIT(EFI_UNSUPPORTED);
}

/**
 * efi_set_watchdog() - resets the watchdog timer
 *
 * This function is used by the SetWatchdogTimer service.
 *
 * @timeout:		seconds before reset by watchdog
 * Return:		status code
 */
efi_status_t efi_set_watchdog(unsigned long timeout)
{
	efi_status_t r;

	if (timeout)
		/* Reset watchdog */
		r = efi_set_timer(watchdog_timer_event, EFI_TIMER_RELATIVE,
				  EFI_SECONDS_TO_100NS * timeout);
	else
		/* Deactivate watchdog */
		r = efi_set_timer(watchdog_timer_event, EFI_TIMER_STOP, 0);
	return r;
}

/**
 * efi_watchdog_register() - initializes the EFI watchdog
 *
 * This function is called by efi_init_obj_list().
 *
 * Return:	status code
 */
efi_status_t efi_watchdog_register(void)
{
	efi_status_t r;

	/*
	 * Create a timer event.
	 */
	r = efi_create_event(EVT_TIMER | EVT_NOTIFY_SIGNAL, TPL_CALLBACK,
			     efi_watchdog_timer_notify, NULL, NULL,
			     &watchdog_timer_event);
	if (r != EFI_SUCCESS) {
		printf("ERROR: Failed to register watchdog event\n");
		return r;
	}

	return EFI_SUCCESS;
}
