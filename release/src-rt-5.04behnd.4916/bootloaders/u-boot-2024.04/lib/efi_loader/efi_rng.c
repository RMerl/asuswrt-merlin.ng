// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2019, Linaro Limited
 */

#define LOG_CATEGORY LOGC_EFI

#include <dm.h>
#include <efi_loader.h>
#include <efi_rng.h>
#include <log.h>
#include <rng.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

const efi_guid_t efi_guid_rng_protocol = EFI_RNG_PROTOCOL_GUID;

/**
 * platform_get_rng_device() - retrieve random number generator
 *
 * This function retrieves the udevice implementing a hardware random
 * number generator.
 *
 * This function may be overridden if special initialization is needed.
 *
 * @dev:	udevice
 * Return:	status code
 */
__weak efi_status_t platform_get_rng_device(struct udevice **dev)
{
	int ret;
	struct udevice *devp;

	ret = uclass_get_device(UCLASS_RNG, 0, &devp);
	if (ret) {
		debug("Unable to get rng device\n");
		return EFI_DEVICE_ERROR;
	}

	*dev = devp;

	return EFI_SUCCESS;
}

/**
 * rng_getinfo() - get information about random number generation
 *
 * This function implement the GetInfo() service of the EFI random number
 * generator protocol. See the UEFI spec for details.
 *
 * @this:			random number generator protocol instance
 * @rng_algorithm_list_size:	number of random number generation algorithms
 * @rng_algorithm_list:		descriptions of random number generation
 *				algorithms
 * Return:			status code
 */
static efi_status_t EFIAPI rng_getinfo(struct efi_rng_protocol *this,
				       efi_uintn_t *rng_algorithm_list_size,
				       efi_guid_t *rng_algorithm_list)
{
	efi_status_t ret = EFI_SUCCESS;
	efi_guid_t rng_algo_guid = EFI_RNG_ALGORITHM_RAW;

	EFI_ENTRY("%p, %p, %p", this, rng_algorithm_list_size,
		  rng_algorithm_list);

	if (!this || !rng_algorithm_list_size) {
		ret = EFI_INVALID_PARAMETER;
		goto back;
	}

	if (!rng_algorithm_list ||
	    *rng_algorithm_list_size < sizeof(*rng_algorithm_list)) {
		*rng_algorithm_list_size = sizeof(*rng_algorithm_list);
		ret = EFI_BUFFER_TOO_SMALL;
		goto back;
	}

	/*
	 * For now, use EFI_RNG_ALGORITHM_RAW as the default
	 * algorithm. If a new algorithm gets added in the
	 * future through a Kconfig, rng_algo_guid will be set
	 * based on that Kconfig option
	 */
	*rng_algorithm_list_size = sizeof(*rng_algorithm_list);
	guidcpy(rng_algorithm_list, &rng_algo_guid);

back:
	return EFI_EXIT(ret);
}

/**
 * rng_getrng() - get random value
 *
 * This function implement the GetRng() service of the EFI random number
 * generator protocol. See the UEFI spec for details.
 *
 * @this:		random number generator protocol instance
 * @rng_algorithm:	random number generation algorithm
 * @rng_value_length:	number of random bytes to generate, buffer length
 * @rng_value:		buffer to receive random bytes
 * Return:		status code
 */
static efi_status_t EFIAPI getrng(struct efi_rng_protocol *this,
				  efi_guid_t *rng_algorithm,
				  efi_uintn_t rng_value_length,
				  uint8_t *rng_value)
{
	int ret;
	efi_status_t status = EFI_SUCCESS;
	struct udevice *dev;
	const efi_guid_t rng_raw_guid = EFI_RNG_ALGORITHM_RAW;

	EFI_ENTRY("%p, %p, %zu, %p", this, rng_algorithm, rng_value_length,
		  rng_value);

	if (!this || !rng_value || !rng_value_length) {
		status = EFI_INVALID_PARAMETER;
		goto back;
	}

	if (rng_algorithm) {
		EFI_PRINT("RNG algorithm %pUs\n", rng_algorithm);
		if (guidcmp(rng_algorithm, &rng_raw_guid)) {
			status = EFI_UNSUPPORTED;
			goto back;
		}
	}

	ret = platform_get_rng_device(&dev);
	if (ret != EFI_SUCCESS) {
		EFI_PRINT("Rng device not found\n");
		status = EFI_UNSUPPORTED;
		goto back;
	}

	ret = dm_rng_read(dev, rng_value, rng_value_length);
	if (ret < 0) {
		EFI_PRINT("Rng device read failed\n");
		status = EFI_DEVICE_ERROR;
		goto back;
	}

back:
	return EFI_EXIT(status);
}

static const struct efi_rng_protocol efi_rng_protocol = {
	.get_info = rng_getinfo,
	.get_rng = getrng,
};

/**
 * efi_rng_register() - register EFI_RNG_PROTOCOL
 *
 * If a RNG device is available, the Random Number Generator Protocol is
 * registered.
 *
 * Return:	An error status is only returned if adding the protocol fails.
 */
efi_status_t efi_rng_register(void)
{
	efi_status_t ret;
	struct udevice *dev;

	ret = platform_get_rng_device(&dev);
	if (ret != EFI_SUCCESS) {
		log_warning("Missing RNG device for EFI_RNG_PROTOCOL\n");
		return EFI_SUCCESS;
	}
	ret = efi_add_protocol(efi_root, &efi_guid_rng_protocol,
			       (void *)&efi_rng_protocol);
	if (ret != EFI_SUCCESS)
		log_err("Cannot install EFI_RNG_PROTOCOL\n");

	return ret;
}
