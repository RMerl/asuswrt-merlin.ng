/*
 * Copyright (C) 2012-2015 Andreas Steffen
 * HSR Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

/**
 * @defgroup imc_os_info imc_os_info
 * @{ @ingroup libimcv_imc
 */

#ifndef IMC_OS_INFO_H_
#define IMC_OS_INFO_H_

typedef struct imc_os_info_t imc_os_info_t;

#include "os_info/os_info.h"

#include <library.h>

#include <time.h>

/**
 * Interface for the IMC Operating System (OS) information module
 */
struct imc_os_info_t {

	/**
	 * Get the OS type if it can be determined
	 *
	 * @return					OS type
	 */
	os_type_t (*get_type)(imc_os_info_t *this);

	/**
	 * Get the OS product name or distribution
	 *
	 * @return					OS name
	 */
	chunk_t (*get_name)(imc_os_info_t *this);

	/**
	 * Get the numeric OS version or release
	 *
	 * @param major				OS major version number
	 * @param minor				OS minor version number
	 */
	void (*get_numeric_version)(imc_os_info_t *this, uint32_t *major,
												 uint32_t *minor);

	/**
	 * Get the OS version or release
	 *
	 * @return					OS version
	 */
	chunk_t (*get_version)(imc_os_info_t *this);

	/**
	 * Get the OS IPv4 forwarding status
	 *
	 * @return					IP forwarding status
	 */
	os_fwd_status_t (*get_fwd_status)(imc_os_info_t *this);

	/**
	 * Get the default password status
	 *
	 * @return					TRUE if enabled, FALSE otherwise
	 */
	bool (*get_default_pwd_status)(imc_os_info_t *this);

	/**
	 * Get the OS uptime in seconds
	 *
	 * @return					OS uptime
	 */
	time_t (*get_uptime)(imc_os_info_t *this);

	/**
	 * Get an OS setting (restricted to /proc, /sys, and /etc)
	 *
	 * @param name				name of OS setting
	 * @return					value of OS setting
	 */
	chunk_t (*get_setting)(imc_os_info_t *this, char *name);

	/**
	 * Enumerates over all installed packages
	 *
	 * @return				return package enumerator
	 */
	enumerator_t* (*create_package_enumerator)(imc_os_info_t *this);

	/**
	 * Destroys an imc_os_info_t object.
	 */
	void (*destroy)(imc_os_info_t *this);
};

/**
 * Create an imc_os_info_t object
 */
imc_os_info_t* imc_os_info_create(void);

#endif /** IMC_OS_INFO_H_ @}*/
