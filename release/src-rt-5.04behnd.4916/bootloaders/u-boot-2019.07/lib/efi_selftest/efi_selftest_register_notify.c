// SPDX-License-Identifier: GPL-2.0+
/*
 * efi_selftest_register_notify
 *
 * Copyright (c) 2019 Heinrich Schuchardt <xypron.glpk@gmx.de>
 *
 * This unit test checks the following protocol services:
 * InstallProtocolInterface, UninstallProtocolInterface,
 * RegisterProtocolNotify, CreateEvent, CloseEvent.
 */

#include <efi_selftest.h>

/*
 * The test currently does not actually call the interface function.
 * So this is just a dummy structure.
 */
struct interface {
	void (EFIAPI * inc)(void);
};

struct context {
	void *registration_key;
	efi_uintn_t notify_count;
	efi_uintn_t handle_count;
	efi_handle_t *handles;
};

static struct efi_boot_services *boottime;
static efi_guid_t guid1 =
	EFI_GUID(0x2e7ca819, 0x21d3, 0x0a3a,
		 0xf7, 0x91, 0x82, 0x1f, 0x7a, 0x83, 0x67, 0xaf);
static efi_guid_t guid2 =
	EFI_GUID(0xf909f2bb, 0x90a8, 0x0d77,
		 0x94, 0x0c, 0x3e, 0xa8, 0xea, 0x38, 0xd6, 0x6f);
static struct context context;
static struct efi_event *event;

/*
 * Notification function, increments the notification count if parameter
 * context is provided.
 *
 * @event	notified event
 * @context	pointer to the notification count
 */
static void EFIAPI notify(struct efi_event *event, void *context)
{
	struct context *cp = context;
	efi_status_t ret;
	efi_uintn_t handle_count;
	efi_handle_t *handles;

	cp->notify_count++;

	for (;;) {
		ret = boottime->locate_handle_buffer(BY_REGISTER_NOTIFY, NULL,
						     cp->registration_key,
						     &handle_count, &handles);
		if (ret != EFI_SUCCESS)
			break;
		cp->handle_count += handle_count;
		cp->handles = handles;
	}
}

/*
 * Setup unit test.
 *
 * @handle:	handle of the loaded image
 * @systable:	system table
 */
static int setup(const efi_handle_t img_handle,
		 const struct efi_system_table *systable)
{
	efi_status_t ret;

	boottime = systable->boottime;

	ret = boottime->create_event(EVT_NOTIFY_SIGNAL,
				     TPL_CALLBACK, notify, &context,
				     &event);
	if (ret != EFI_SUCCESS) {
		efi_st_error("could not create event\n");
		return EFI_ST_FAILURE;
	}

	ret = boottime->register_protocol_notify(&guid1, event,
						 &context.registration_key);
	if (ret != EFI_SUCCESS) {
		efi_st_error("could not register event\n");
		return EFI_ST_FAILURE;
	}

	return EFI_ST_SUCCESS;
}

/*
 * Tear down unit test.
 *
 */
static int teardown(void)
{
	efi_status_t ret;

	if (event) {
		ret = boottime->close_event(event);
		event = NULL;
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
 */
static int execute(void)
{
	efi_status_t ret;
	efi_handle_t handle1 = NULL, handle2 = NULL;
	struct interface interface1, interface2;

	ret = boottime->install_protocol_interface(&handle1, &guid1,
						   EFI_NATIVE_INTERFACE,
						   &interface1);
	if (ret != EFI_SUCCESS) {
		efi_st_error("could not install interface\n");
		return EFI_ST_FAILURE;
	}
	if (!context.notify_count) {
		efi_st_error("install was not notified\n");
		return EFI_ST_FAILURE;
	}
	if (context.notify_count > 1) {
		efi_st_error("install was notified too often\n");
		return EFI_ST_FAILURE;
	}
	if (context.handle_count != 1) {
		efi_st_error("LocateHandle failed\n");
		return EFI_ST_FAILURE;
	}
	ret = boottime->free_pool(context.handles);
	if (ret != EFI_SUCCESS) {
		efi_st_error("FreePool failed\n");
		return EFI_ST_FAILURE;
	}
	context.notify_count = 0;
	ret = boottime->install_protocol_interface(&handle1, &guid2,
						   EFI_NATIVE_INTERFACE,
						   &interface1);
	if (ret != EFI_SUCCESS) {
		efi_st_error("could not install interface\n");
		return EFI_ST_FAILURE;
	}
	if (context.notify_count) {
		efi_st_error("wrong protocol was notified\n");
		return EFI_ST_FAILURE;
	}
	context.notify_count = 0;
	ret = boottime->reinstall_protocol_interface(handle1, &guid1,
						     &interface1, &interface2);
	if (ret != EFI_SUCCESS) {
		efi_st_error("could not reinstall interface\n");
		return EFI_ST_FAILURE;
	}
	if (!context.notify_count) {
		efi_st_error("reinstall was not notified\n");
		return EFI_ST_FAILURE;
	}
	if (context.notify_count > 1) {
		efi_st_error("reinstall was notified too often\n");
		return EFI_ST_FAILURE;
	}
	if (context.handle_count != 2) {
		efi_st_error("LocateHandle failed\n");
		return EFI_ST_FAILURE;
	}
	ret = boottime->free_pool(context.handles);
	if (ret != EFI_SUCCESS) {
		efi_st_error("FreePool failed\n");
		return EFI_ST_FAILURE;
	}
	context.notify_count = 0;
	ret = boottime->install_protocol_interface(&handle2, &guid1,
						   EFI_NATIVE_INTERFACE,
						   &interface1);
	if (ret != EFI_SUCCESS) {
		efi_st_error("could not install interface\n");
		return EFI_ST_FAILURE;
	}
	if (!context.notify_count) {
		efi_st_error("install was not notified\n");
		return EFI_ST_FAILURE;
	}
	if (context.notify_count > 1) {
		efi_st_error("install was notified too often\n");
		return EFI_ST_FAILURE;
	}
	if (context.handle_count != 3) {
		efi_st_error("LocateHandle failed\n");
		return EFI_ST_FAILURE;
	}
	ret = boottime->free_pool(context.handles);
	if (ret != EFI_SUCCESS) {
		efi_st_error("FreePool failed\n");
		return EFI_ST_FAILURE;
	}

	ret = boottime->uninstall_multiple_protocol_interfaces
			(handle1, &guid1, &interface2,
			 &guid2, &interface1, NULL);
	if (ret != EFI_SUCCESS) {
		efi_st_error("UninstallMultipleProtocolInterfaces failed\n");
		return EFI_ST_FAILURE;
	}
	ret = boottime->uninstall_multiple_protocol_interfaces
			(handle2, &guid1, &interface1, NULL);
	if (ret != EFI_SUCCESS) {
		efi_st_error("UninstallMultipleProtocolInterfaces failed\n");
		return EFI_ST_FAILURE;
	}

	return EFI_ST_SUCCESS;
}

EFI_UNIT_TEST(regprotnot) = {
	.name = "register protocol notify",
	.phase = EFI_EXECUTE_BEFORE_BOOTTIME_EXIT,
	.setup = setup,
	.execute = execute,
	.teardown = teardown,
};
