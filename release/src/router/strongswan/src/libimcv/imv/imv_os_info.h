/*
 * Copyright (C) 2014 Andreas Steffen
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
 * @defgroup imv_os_info imv_os_info
 * @{ @ingroup libimcv_imv
 */

#ifndef IMV_OS_INFO_H_
#define IMV_OS_INFO_H_

typedef struct imv_os_info_t imv_os_info_t;

#include "os_info/os_info.h"

#include <library.h>

/**
 * Interface for the IMV Operating System (OS) information module
 */
struct imv_os_info_t {

	/**
	 * Get the OS type
	 *
	 * @return					OS type
	 */
	os_type_t (*get_type)(imv_os_info_t *this);

	/**
	 * Set the OS product name or distribution
	 *
	 * @param name				OS name
	 */
	void (*set_name)(imv_os_info_t *this, chunk_t name);

	/**
	 * Get the OS product name or distribution
	 *
	 * @return					OS name
	 */
	chunk_t (*get_name)(imv_os_info_t *this);

	/**
	 * Set the OS version or release
	 *
	 * @param version			OS version
	 */
	void (*set_version)(imv_os_info_t *this, chunk_t version);

	/**
	 * Get the OS version or release
	 *
	 * @return					OS version
	 */
	chunk_t (*get_version)(imv_os_info_t *this);

	/**
	 * Get the OS version or release
	 *
	 * @return					OS name | OS version
	 */
	char* (*get_info)(imv_os_info_t *this);

	/**
	 * Destroys an imv_os_info_t object.
	 */
	void (*destroy)(imv_os_info_t *this);
};

/**
 * Create an imv_os_info_t object
 */
imv_os_info_t* imv_os_info_create(void);

#endif /** IMV_OS_INFO_H_ @}*/
