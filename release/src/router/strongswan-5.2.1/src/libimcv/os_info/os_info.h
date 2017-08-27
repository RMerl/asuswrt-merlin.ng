/*
 * Copyright (C) 2012-2014 Andreas Steffen
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
 * @defgroup os_info os_info
 * @{ @ingroup libimcv
 */

#ifndef OS_INFO_H_
#define OS_INFO_H_

typedef enum os_type_t os_type_t;
typedef enum os_fwd_status_t os_fwd_status_t;
typedef enum os_package_state_t os_package_state_t;

#include <library.h>

#include <time.h>

enum os_type_t {
	OS_TYPE_UNKNOWN,
	OS_TYPE_DEBIAN,
	OS_TYPE_UBUNTU,
	OS_TYPE_FEDORA,
	OS_TYPE_REDHAT,
	OS_TYPE_CENTOS,
	OS_TYPE_SUSE,
	OS_TYPE_GENTOO,
	OS_TYPE_ANDROID,
	OS_TYPE_WINDOWS,
	OS_TYPE_ROOF
};

extern enum_name_t *os_type_names;

/**
 * Defines the security state of a package stored in the database
 */
enum os_package_state_t {
	OS_PACKAGE_STATE_UPDATE =    0,		/* latest update */
	OS_PACKAGE_STATE_SECURITY =  1,		/* latest security fix */
	OS_PACKAGE_STATE_BLACKLIST = 2 		/* blacklisted package */
};

extern enum_name_t *os_package_state_names;

/**
 * Defines the IPv4 forwarding status
 */
enum os_fwd_status_t {
	OS_FWD_DISABLED =	0,
	OS_FWD_ENABLED =	1,
	OS_FWD_UNKNOWN =	2
};

extern enum_name_t *os_fwd_status_names;

/**
 * Convert an OS name into an OS enumeration type
 *
 * @param name							OS name
 * @return								OS enumeration type
 */
os_type_t os_type_from_name(chunk_t name);

#endif /** OS_INFO_H_ @}*/
