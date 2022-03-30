// SPDX-License-Identifier: GPL-2.0+
/*
 *  Uclass for EFI drivers
 *
 *  Copyright (c) 2017 Heinrich Schuchardt
 *
 * For each EFI driver the uclass
 * - creates a handle
 * - installs the driver binding protocol
 *
 * The uclass provides the bind, start, and stop entry points for the driver
 * binding protocol.
 *
 * In bind() and stop() it checks if the controller implements the protocol
 * supported by the EFI driver. In the start() function it calls the bind()
 * function of the EFI driver. In the stop() function it destroys the child
 * controllers.
 */

#include <efi_driver.h>

/**
 * check_node_type() - check node type
 *
 * We do not support partitions as controller handles.
 *
 * @handle:	handle to be checked
 * Return:	status code
 */
static efi_status_t check_node_type(efi_handle_t handle)
{
	efi_status_t r, ret = EFI_SUCCESS;
	const struct efi_device_path *dp;

	/* Open the device path protocol */
	r = EFI_CALL(systab.boottime->open_protocol(
			handle, &efi_guid_device_path, (void **)&dp,
			NULL, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL));
	if (r == EFI_SUCCESS && dp) {
		/* Get the last node */
		const struct efi_device_path *node = efi_dp_last_node(dp);
		/* We do not support partitions as controller */
		if (!node || node->type == DEVICE_PATH_TYPE_MEDIA_DEVICE)
			ret = EFI_UNSUPPORTED;
	}
	return ret;
}

/**
 * efi_uc_supported() - check if the driver supports the controller
 *
 * @this:			driver binding protocol
 * @controller_handle:		handle of the controller
 * @remaining_device_path:	path specifying the child controller
 * Return:			status code
 */
static efi_status_t EFIAPI efi_uc_supported(
		struct efi_driver_binding_protocol *this,
		efi_handle_t controller_handle,
		struct efi_device_path *remaining_device_path)
{
	efi_status_t r, ret;
	void *interface;
	struct efi_driver_binding_extended_protocol *bp =
			(struct efi_driver_binding_extended_protocol *)this;

	EFI_ENTRY("%p, %p, %ls", this, controller_handle,
		  efi_dp_str(remaining_device_path));

	ret = EFI_CALL(systab.boottime->open_protocol(
			controller_handle, bp->ops->protocol,
			&interface, this->driver_binding_handle,
			controller_handle, EFI_OPEN_PROTOCOL_BY_DRIVER));
	switch (ret) {
	case EFI_ACCESS_DENIED:
	case EFI_ALREADY_STARTED:
		goto out;
	case EFI_SUCCESS:
		break;
	default:
		ret = EFI_UNSUPPORTED;
		goto out;
	}

	ret = check_node_type(controller_handle);

	r = EFI_CALL(systab.boottime->close_protocol(
				controller_handle, bp->ops->protocol,
				this->driver_binding_handle,
				controller_handle));
	if (r != EFI_SUCCESS)
		ret = EFI_UNSUPPORTED;
out:
	return EFI_EXIT(ret);
}

/**
 * efi_uc_start() - create child controllers and attach driver
 *
 * @this:			driver binding protocol
 * @controller_handle:		handle of the controller
 * @remaining_device_path:	path specifying the child controller
 * Return:			status code
 */
static efi_status_t EFIAPI efi_uc_start(
		struct efi_driver_binding_protocol *this,
		efi_handle_t controller_handle,
		struct efi_device_path *remaining_device_path)
{
	efi_status_t r, ret;
	void *interface = NULL;
	struct efi_driver_binding_extended_protocol *bp =
			(struct efi_driver_binding_extended_protocol *)this;

	EFI_ENTRY("%p, %pUl, %ls", this, controller_handle,
		  efi_dp_str(remaining_device_path));

	/* Attach driver to controller */
	ret = EFI_CALL(systab.boottime->open_protocol(
			controller_handle, bp->ops->protocol,
			&interface, this->driver_binding_handle,
			controller_handle, EFI_OPEN_PROTOCOL_BY_DRIVER));
	switch (ret) {
	case EFI_ACCESS_DENIED:
	case EFI_ALREADY_STARTED:
		goto out;
	case EFI_SUCCESS:
		break;
	default:
		ret =  EFI_UNSUPPORTED;
		goto out;
	}
	ret = check_node_type(controller_handle);
	if (ret != EFI_SUCCESS) {
		r = EFI_CALL(systab.boottime->close_protocol(
				controller_handle, bp->ops->protocol,
				this->driver_binding_handle,
				controller_handle));
		if (r != EFI_SUCCESS)
			EFI_PRINT("Failure to close handle\n");
		goto out;
	}

	/* TODO: driver specific stuff */
	bp->ops->bind(controller_handle, interface);

out:
	return EFI_EXIT(ret);
}

/**
 * disconnect_child() - remove a single child controller from the parent
 *			controller
 *
 * @controller_handle:	parent controller
 * @child_handle:	child controller
 * Return:		status code
 */
static efi_status_t disconnect_child(efi_handle_t controller_handle,
				     efi_handle_t child_handle)
{
	efi_status_t ret;
	efi_guid_t *guid_controller = NULL;
	efi_guid_t *guid_child_controller = NULL;

	ret = EFI_CALL(systab.boottime->close_protocol(
				controller_handle, guid_controller,
				child_handle, child_handle));
	if (ret != EFI_SUCCESS) {
		EFI_PRINT("Cannot close protocol\n");
		return ret;
	}
	ret = EFI_CALL(systab.boottime->uninstall_protocol_interface(
				child_handle, guid_child_controller, NULL));
	if (ret != EFI_SUCCESS) {
		EFI_PRINT("Cannot uninstall protocol interface\n");
		return ret;
	}
	return ret;
}

/**
 * efi_uc_stop() - Remove child controllers and disconnect the controller
 *
 * @this:			driver binding protocol
 * @controller_handle:		handle of the controller
 * @number_of_children:		number of child controllers to remove
 * @child_handle_buffer:	handles of the child controllers to remove
 * Return:			status code
 */
static efi_status_t EFIAPI efi_uc_stop(
		struct efi_driver_binding_protocol *this,
		efi_handle_t controller_handle,
		size_t number_of_children,
		efi_handle_t *child_handle_buffer)
{
	efi_status_t ret;
	efi_uintn_t count;
	struct efi_open_protocol_info_entry *entry_buffer;
	efi_guid_t *guid_controller = NULL;

	EFI_ENTRY("%p, %pUl, %zu, %p", this, controller_handle,
		  number_of_children, child_handle_buffer);

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
	ret = EFI_CALL(systab.boottime->open_protocol_information(
					controller_handle, guid_controller,
					&entry_buffer, &count));
	if (ret != EFI_SUCCESS)
		goto out;
	while (count) {
		if (entry_buffer[--count].attributes &
		    EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER) {
			ret = disconnect_child(
					controller_handle,
					entry_buffer[count].agent_handle);
			if (ret != EFI_SUCCESS)
				goto out;
		}
	}
	ret = EFI_CALL(systab.boottime->free_pool(entry_buffer));
	if (ret != EFI_SUCCESS)
		printf("%s: ERROR: Cannot free pool\n", __func__);

	/* Detach driver from controller */
	ret = EFI_CALL(systab.boottime->close_protocol(
			controller_handle, guid_controller,
			this->driver_binding_handle, controller_handle));
out:
	return EFI_EXIT(ret);
}

/**
 * efi_add_driver() - add driver
 *
 * @drv:		driver to add
 * Return:		status code
 */
static efi_status_t efi_add_driver(struct driver *drv)
{
	efi_status_t ret;
	const struct efi_driver_ops *ops = drv->ops;
	struct efi_driver_binding_extended_protocol *bp;

	debug("EFI: Adding driver '%s'\n", drv->name);
	if (!ops->protocol) {
		printf("EFI: ERROR: protocol GUID missing for driver '%s'\n",
		       drv->name);
		return EFI_INVALID_PARAMETER;
	}
	bp = calloc(1, sizeof(struct efi_driver_binding_extended_protocol));
	if (!bp)
		return EFI_OUT_OF_RESOURCES;

	bp->bp.supported = efi_uc_supported;
	bp->bp.start = efi_uc_start;
	bp->bp.stop = efi_uc_stop;
	bp->bp.version = 0xffffffff;
	bp->ops = drv->ops;

	ret = efi_create_handle(&bp->bp.driver_binding_handle);
	if (ret != EFI_SUCCESS) {
		free(bp);
		goto out;
	}
	bp->bp.image_handle = bp->bp.driver_binding_handle;
	ret = efi_add_protocol(bp->bp.driver_binding_handle,
			       &efi_guid_driver_binding_protocol, bp);
	if (ret != EFI_SUCCESS) {
		efi_delete_handle(bp->bp.driver_binding_handle);
		free(bp);
		goto out;
	}
out:
	return ret;
}

/**
 * efi_driver_init() - initialize the EFI drivers
 *
 * Called by efi_init_obj_list().
 *
 * Return:	0 = success, any other value will stop further execution
 */
efi_status_t efi_driver_init(void)
{
	struct driver *drv;
	efi_status_t ret = EFI_SUCCESS;

	debug("EFI: Initializing EFI driver framework\n");
	for (drv = ll_entry_start(struct driver, driver);
	     drv < ll_entry_end(struct driver, driver); ++drv) {
		if (drv->id == UCLASS_EFI) {
			ret = efi_add_driver(drv);
			if (ret != EFI_SUCCESS) {
				printf("EFI: ERROR: failed to add driver %s\n",
				       drv->name);
				break;
			}
		}
	}
	return ret;
}

/**
 * efi_uc_init() - initialize the EFI uclass
 *
 * @class:	the EFI uclass
 * Return:	0 = success
 */
static int efi_uc_init(struct uclass *class)
{
	printf("EFI: Initializing UCLASS_EFI\n");
	return 0;
}

/**
 * efi_uc_destroy() - destroy the EFI uclass
 *
 * @class:	the EFI uclass
 * Return:	0 = success
 */
static int efi_uc_destroy(struct uclass *class)
{
	printf("Destroying  UCLASS_EFI\n");
	return 0;
}

UCLASS_DRIVER(efi) = {
	.name		= "efi",
	.id		= UCLASS_EFI,
	.init		= efi_uc_init,
	.destroy	= efi_uc_destroy,
};
