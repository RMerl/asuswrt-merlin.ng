// SPDX-License-Identifier: GPL-2.0+
/*
 *  EFI watchdog
 *
 *  Copyright (c) 2017 Heinrich Schuchardt
 */

#include <common.h>
#include <efi_loader.h>

/* Conversion factor from seconds to multiples of 100ns */
#define EFI_SECONDS_TO_100NS 10000000ULL

static struct efi_event *watchdog_timer_event;

/*
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
	EFI_CALL_VOID(efi_runtime_services.reset_system(EFI_RESET_COLD,
							EFI_SUCCESS, 0, NULL));

	EFI_EXIT(EFI_UNSUPPORTED);
}

/*
 * Reset the watchdog timer.
 *
 * This function is used by the SetWatchdogTimer service.
 *
 * @timeout:		seconds before reset by watchdog
 * @return:		status code
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

/*
 * Initialize the EFI watchdog.
 *
 * This function is called by efi_init_obj_list()
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
	/*
	 * The UEFI standard requires that the watchdog timer is set to five
	 * minutes when invoking an EFI boot option.
	 *
	 * Unified Extensible Firmware Interface (UEFI), version 2.7 Errata A
	 * 7.5. Miscellaneous Boot Services - EFI_BOOT_SERVICES.SetWatchdogTimer
	 */
	r = efi_set_watchdog(300);
	if (r != EFI_SUCCESS) {
		printf("ERROR: Failed to set watchdog timer\n");
		return r;
	}
	return EFI_SUCCESS;
}
