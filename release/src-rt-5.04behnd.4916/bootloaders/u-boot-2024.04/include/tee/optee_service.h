/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * (C) Copyright 2022 Linaro Limited
 */

#ifndef _OPTEE_SERVICE_H
#define _OPTEE_SERVICE_H

/*
 * struct optee_service - Discoverable OP-TEE service
 *
 * @driver_name - Name of the related driver
 * @uuid - UUID of the OP-TEE service related to the driver
 *
 * Use macro OPTEE_SERVICE_DRIVER() to register a driver related to an
 * OP-TEE service discovered when driver asks OP-TEE services enumaration.
 */
struct optee_service {
	const char *driver_name;
	const struct tee_optee_ta_uuid uuid;
};

#ifdef CONFIG_OPTEE_SERVICE_DISCOVERY
#define OPTEE_SERVICE_DRIVER(__name, __uuid, __drv_name) \
	ll_entry_declare(struct optee_service, __name, optee_service) = { \
		.uuid = __uuid, \
		.driver_name = __drv_name, \
	}
#else
#define OPTEE_SERVICE_DRIVER(__name, __uuid, __drv_name) \
	static int __name##__COUNTER__ __always_unused
#endif

#endif /* _OPTEE_SERVICE_H */
