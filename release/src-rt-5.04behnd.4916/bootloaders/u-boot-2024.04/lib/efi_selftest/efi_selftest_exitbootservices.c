// SPDX-License-Identifier: GPL-2.0+
/*
 * efi_selftest_exitbootservices
 *
 * Copyright (c) 2017 Heinrich Schuchardt <xypron.glpk@gmx.de>
 *
 * This unit test checks that the notification function of an
 * EVT_SIGNAL_EXIT_BOOT_SERVICES event is called exactly once.
 */

#include <efi_selftest.h>

static efi_guid_t guid_before_exit_boot_services =
	EFI_GUID(0x8be0e274, 0x3970, 0x4b44, 0x80, 0xc5,
		 0x1a, 0xb9, 0x50, 0x2f, 0x3b, 0xfc);
#define CAPACITY 4

struct notification_record {
	unsigned int count;
	unsigned int type[CAPACITY];
};

struct notification_context {
	struct notification_record *record;
	unsigned int type;
};

static struct efi_boot_services *boottime;
static struct efi_event *efi_st_event_notify;
static struct notification_record record;

struct notification_context context_before = {
	.record = &record,
	.type = 1,
};

struct notification_context context = {
	.record = &record,
	.type = 2,
};

/*
 * Notification function, increments the notification count.
 *
 * @event	notified event
 * @context	pointer to the notification count
 */
static void EFIAPI ebs_notify(struct efi_event *event, void *context)
{
	struct notification_context *ctx = context;

	if (ctx->record->count >= CAPACITY)
		return;

	ctx->record->type[ctx->record->count] = ctx->type;
	ctx->record->count++;
}

/*
 * Setup unit test.
 *
 * Create an EVT_SIGNAL_EXIT_BOOT_SERVICES event.
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

	ret = boottime->create_event(EVT_SIGNAL_EXIT_BOOT_SERVICES,
				     TPL_CALLBACK, ebs_notify,
				     &context,
				     &efi_st_event_notify);
	if (ret != EFI_SUCCESS) {
		efi_st_error("could not create event\n");
		return EFI_ST_FAILURE;
	}
	ret = boottime->create_event_ex(0, TPL_CALLBACK, ebs_notify,
					&context_before,
					&guid_before_exit_boot_services,
					&efi_st_event_notify);
	if (ret != EFI_SUCCESS) {
		efi_st_error("could not create event\n");
		return EFI_ST_FAILURE;
	}

	return EFI_ST_SUCCESS;
}

/*
 * Execute unit test.
 *
 * Check that the notification function of the EVT_SIGNAL_EXIT_BOOT_SERVICES
 * event has been called.
 *
 * Call ExitBootServices again and check that the notification function is
 * not called again.
 *
 * Return:	EFI_ST_SUCCESS for success
 */
static int execute(void)
{
	if (record.count != 2) {
		efi_st_error("Incorrect event count %u\n", record.count);
		return EFI_ST_FAILURE;
	}
	if (record.type[0] != 1) {
		efi_st_error("EFI_GROUP_BEFORE_EXIT_BOOT_SERVICE not notified\n");
		return EFI_ST_FAILURE;
	}
	if (record.type[1] != 2) {
		efi_st_error("EVT_SIGNAL_EXIT_BOOT_SERVICES was not notified\n");
		return EFI_ST_FAILURE;
	}
	efi_st_exit_boot_services();
	if (record.count != 2) {
		efi_st_error("Incorrect event count %u\n", record.count);
		return EFI_ST_FAILURE;
	}
	return EFI_ST_SUCCESS;
}

EFI_UNIT_TEST(exitbootservices) = {
	.name = "ExitBootServices",
	.phase = EFI_SETUP_BEFORE_BOOTTIME_EXIT,
	.setup = setup,
	.execute = execute,
};
