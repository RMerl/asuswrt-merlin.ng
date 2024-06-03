// SPDX-License-Identifier: GPL-2.0+
/*
 * efi_selftest_controllers
 *
 * Copyright (c) 2017 Heinrich Schuchardt <xypron.glpk@gmx.de>
 *
 * This unit test checks the following protocol services:
 * ConnectController, DisconnectController,
 * InstallProtocol, ReinstallProtocol, UninstallProtocol,
 * OpenProtocol, CloseProtcol, OpenProtocolInformation
 */

#include <efi_selftest.h>

#define NUMBER_OF_CHILD_CONTROLLERS 4

static int interface1 = 1;
static int interface2 = 2;
static struct efi_boot_services *boottime;
const efi_guid_t guid_driver_binding_protocol =
			EFI_DRIVER_BINDING_PROTOCOL_GUID;
static efi_guid_t guid_controller =
	EFI_GUID(0xe6ab1d96, 0x6bff, 0xdb42,
		 0xaa, 0x05, 0xc8, 0x1f, 0x7f, 0x45, 0x26, 0x34);
static efi_guid_t guid_child_controller =
	EFI_GUID(0x1d41f6f5, 0x2c41, 0xddfb,
		 0xe2, 0x9b, 0xb8, 0x0e, 0x2e, 0xe8, 0x3a, 0x85);
static efi_handle_t handle_controller;
static efi_handle_t handle_child_controller[NUMBER_OF_CHILD_CONTROLLERS];
static efi_handle_t handle_driver;

/*
 * Count child controllers
 *
 * @handle	handle on which child controllers are installed
 * @protocol	protocol for which the child controllers were installed
 * @count	number of child controllers
 * @return	status code
 */
static efi_status_t count_child_controllers(efi_handle_t handle,
					    efi_guid_t *protocol,
					    efi_uintn_t *count)
{
	efi_status_t ret;
	efi_uintn_t entry_count;
	struct efi_open_protocol_info_entry *entry_buffer;

	*count = 0;
	ret = boottime->open_protocol_information(handle, protocol,
						  &entry_buffer, &entry_count);
	if (ret != EFI_SUCCESS)
		return ret;
	if (!entry_count)
		return EFI_SUCCESS;
	while (entry_count) {
		if (entry_buffer[--entry_count].attributes &
		    EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER)
			++*count;
	}
	ret = boottime->free_pool(entry_buffer);
	if (ret != EFI_SUCCESS)
		efi_st_error("Cannot free buffer\n");
	return ret;
}

/*
 * Check if the driver supports the controller.
 *
 * @this			driver binding protocol
 * @controller_handle		handle of the controller
 * @remaining_device_path	path specifying the child controller
 * @return			status code
 */
static efi_status_t EFIAPI supported(
		struct efi_driver_binding_protocol *this,
		efi_handle_t controller_handle,
		struct efi_device_path *remaining_device_path)
{
	efi_status_t ret;
	void *interface;

	ret = boottime->open_protocol(
			controller_handle, &guid_controller,
			&interface, handle_driver,
			controller_handle, EFI_OPEN_PROTOCOL_BY_DRIVER);
	switch (ret) {
	case EFI_ACCESS_DENIED:
	case EFI_ALREADY_STARTED:
		return ret;
	case EFI_SUCCESS:
		break;
	default:
		return EFI_UNSUPPORTED;
	}
	ret = boottime->close_protocol(
				controller_handle, &guid_controller,
				handle_driver, controller_handle);
	if (ret != EFI_SUCCESS)
		ret = EFI_UNSUPPORTED;
	return ret;
}

/*
 * Create child controllers and attach driver.
 *
 * @this			driver binding protocol
 * @controller_handle		handle of the controller
 * @remaining_device_path	path specifying the child controller
 * @return			status code
 */
static efi_status_t EFIAPI start(
		struct efi_driver_binding_protocol *this,
		efi_handle_t controller_handle,
		struct efi_device_path *remaining_device_path)
{
	size_t i;
	efi_status_t ret;
	void *interface;

	/* Attach driver to controller */
	ret = boottime->open_protocol(
			controller_handle, &guid_controller,
			&interface, handle_driver,
			controller_handle, EFI_OPEN_PROTOCOL_BY_DRIVER);
	switch (ret) {
	case EFI_ACCESS_DENIED:
	case EFI_ALREADY_STARTED:
		return ret;
	case EFI_SUCCESS:
		break;
	default:
		return EFI_UNSUPPORTED;
	}

	/* Create child controllers */
	for (i = 0; i < NUMBER_OF_CHILD_CONTROLLERS; ++i) {
		/* Creating a new handle for the child controller */
		handle_child_controller[i] = 0;
		ret = boottime->install_protocol_interface(
			&handle_child_controller[i], &guid_child_controller,
			EFI_NATIVE_INTERFACE, NULL);
		if (ret != EFI_SUCCESS) {
			efi_st_error("InstallProtocolInterface failed\n");
			return EFI_ST_FAILURE;
		}
		ret = boottime->open_protocol(
			controller_handle, &guid_controller,
			&interface, handle_child_controller[i],
			handle_child_controller[i],
			EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER);
		if (ret != EFI_SUCCESS) {
			efi_st_error("OpenProtocol failed\n");
			return EFI_ST_FAILURE;
		}
	}
	return ret;
}

/*
 * Remove a single child controller from the parent controller.
 *
 * @controller_handle	parent controller
 * @child_handle	child controller
 * @return		status code
 */
static efi_status_t disconnect_child(efi_handle_t controller_handle,
				     efi_handle_t child_handle)
{
	efi_status_t ret;

	ret = boottime->close_protocol(
				controller_handle, &guid_controller,
				child_handle, child_handle);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Cannot close protocol\n");
		return ret;
	}
	ret = boottime->uninstall_protocol_interface(
				child_handle, &guid_child_controller, NULL);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Cannot uninstall protocol interface\n");
		return ret;
	}
	return ret;
}

/*
 * Remove child controllers and disconnect the controller.
 *
 * @this			driver binding protocol
 * @controller_handle		handle of the controller
 * @number_of_children		number of child controllers to remove
 * @child_handle_buffer		handles of the child controllers to remove
 * @return			status code
 */
static efi_status_t EFIAPI stop(
		struct efi_driver_binding_protocol *this,
		efi_handle_t controller_handle,
		size_t number_of_children,
		efi_handle_t *child_handle_buffer)
{
	efi_status_t ret;
	efi_uintn_t count;
	struct efi_open_protocol_info_entry *entry_buffer;

	/* Destroy provided child controllers */
	if (number_of_children) {
		efi_uintn_t i;

		for (i = 0; i < number_of_children; ++i) {
			ret = disconnect_child(controller_handle,
					       child_handle_buffer[i]);
			if (ret != EFI_SUCCESS)
				return ret;
		}
		return EFI_SUCCESS;
	}

	/* Destroy all children */
	ret = boottime->open_protocol_information(
					controller_handle, &guid_controller,
					&entry_buffer, &count);
	if (ret != EFI_SUCCESS) {
		efi_st_error("OpenProtocolInformation failed\n");
		return ret;
	}
	while (count) {
		if (entry_buffer[--count].attributes &
		    EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER) {
			ret = disconnect_child(
					controller_handle,
					entry_buffer[count].agent_handle);
			if (ret != EFI_SUCCESS)
				return ret;
		}
	}
	ret = boottime->free_pool(entry_buffer);
	if (ret != EFI_SUCCESS)
		efi_st_error("Cannot free buffer\n");

	/* Detach driver from controller */
	ret = boottime->close_protocol(
			controller_handle, &guid_controller,
			handle_driver, controller_handle);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Cannot close protocol\n");
		return ret;
	}
	return EFI_SUCCESS;
}

/* Driver binding protocol interface */
static struct efi_driver_binding_protocol binding_interface = {
	supported,
	start,
	stop,
	0xffffffff,
	NULL,
	NULL,
	};

/*
 * Setup unit test.
 *
 * @handle	handle of the loaded image
 * @systable	system table
 */
static int setup(const efi_handle_t img_handle,
		 const struct efi_system_table *systable)
{
	efi_status_t ret;

	boottime = systable->boottime;

	/* Create controller handle */
	ret = boottime->install_protocol_interface(
			&handle_controller, &guid_controller,
			EFI_NATIVE_INTERFACE, &interface1);
	if (ret != EFI_SUCCESS) {
		efi_st_error("InstallProtocolInterface failed\n");
		return EFI_ST_FAILURE;
	}
	/* Create driver handle */
	ret = boottime->install_protocol_interface(
			&handle_driver,  &guid_driver_binding_protocol,
			EFI_NATIVE_INTERFACE, &binding_interface);
	if (ret != EFI_SUCCESS) {
		efi_st_error("InstallProtocolInterface failed\n");
		return EFI_ST_FAILURE;
	}

	return EFI_ST_SUCCESS;
}

/*
 * Execute unit test.
 *
 * The number of child controllers is checked after each of the following
 * actions:
 *
 * Connect a controller to a driver.
 * Disconnect and destroy a child controller.
 * Disconnect and destroy the remaining child controllers.
 *
 * Connect a controller to a driver.
 * Reinstall the driver protocol on the controller.
 * Uninstall the driver protocol from the controller.
 */
static int execute(void)
{
	efi_status_t ret;
	efi_uintn_t count;

	/* Connect controller to driver */
	ret = boottime->connect_controller(handle_controller, NULL, NULL, 1);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Failed to connect controller\n");
		return EFI_ST_FAILURE;
	}
	/* Check number of child controllers */
	ret = count_child_controllers(handle_controller, &guid_controller,
				      &count);
	if (ret != EFI_SUCCESS || count != NUMBER_OF_CHILD_CONTROLLERS) {
		efi_st_error("Number of children %u != %u\n",
			     (unsigned int)count, NUMBER_OF_CHILD_CONTROLLERS);
	}
	/* Destroy second child controller */
	ret = boottime->disconnect_controller(handle_controller,
					      handle_driver,
					      handle_child_controller[1]);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Failed to disconnect child controller\n");
		return EFI_ST_FAILURE;
	}
	/* Check number of child controllers */
	ret = count_child_controllers(handle_controller, &guid_controller,
				      &count);
	if (ret != EFI_SUCCESS || count != NUMBER_OF_CHILD_CONTROLLERS - 1) {
		efi_st_error("Destroying single child controller failed\n");
		return EFI_ST_FAILURE;
	}
	/* Destroy remaining child controllers and disconnect controller */
	ret = boottime->disconnect_controller(handle_controller, NULL, NULL);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Failed to disconnect controller\n");
		return EFI_ST_FAILURE;
	}
	/* Check number of child controllers */
	ret = count_child_controllers(handle_controller, &guid_controller,
				      &count);
	if (ret != EFI_SUCCESS || count) {
		efi_st_error("Destroying child controllers failed\n");
		return EFI_ST_FAILURE;
	}

	/* Connect controller to driver */
	ret = boottime->connect_controller(handle_controller, NULL, NULL, 1);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Failed to connect controller\n");
		return EFI_ST_FAILURE;
	}
	/* Check number of child controllers */
	ret = count_child_controllers(handle_controller, &guid_controller,
				      &count);
	if (ret != EFI_SUCCESS || count != NUMBER_OF_CHILD_CONTROLLERS) {
		efi_st_error("Number of children %u != %u\n",
			     (unsigned int)count, NUMBER_OF_CHILD_CONTROLLERS);
	}
	/* Try to uninstall controller protocol using the wrong interface */
	ret = boottime->uninstall_protocol_interface(handle_controller,
						     &guid_controller,
						     &interface2);
	if (ret == EFI_SUCCESS) {
		efi_st_error(
			"Interface not checked when uninstalling protocol\n");
		return EFI_ST_FAILURE;
	}
	/* Reinstall controller protocol */
	ret = boottime->reinstall_protocol_interface(handle_controller,
						     &guid_controller,
						     &interface1,
						     &interface2);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Failed to reinstall protocols\n");
		return EFI_ST_FAILURE;
	}
	/* Check number of child controllers */
	ret = count_child_controllers(handle_controller, &guid_controller,
				      &count);
	if (ret != EFI_SUCCESS || count != NUMBER_OF_CHILD_CONTROLLERS) {
		efi_st_error("Number of children %u != %u\n",
			     (unsigned int)count, NUMBER_OF_CHILD_CONTROLLERS);
	}
	/* Uninstall controller protocol */
	ret = boottime->uninstall_protocol_interface(handle_controller,
						     &guid_controller,
						     &interface2);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Failed to uninstall protocols\n");
		return EFI_ST_FAILURE;
	}
	/* Check number of child controllers */
	ret = count_child_controllers(handle_controller, &guid_controller,
				      &count);
	if (ret == EFI_SUCCESS)
		efi_st_error("Uninstall failed\n");
	return EFI_ST_SUCCESS;
}

EFI_UNIT_TEST(controllers) = {
	.name = "controllers",
	.phase = EFI_EXECUTE_BEFORE_BOOTTIME_EXIT,
	.setup = setup,
	.execute = execute,
};
