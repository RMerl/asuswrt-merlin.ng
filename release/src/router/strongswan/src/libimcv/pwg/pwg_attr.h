/*
 * Copyright (C) 2015 Andreas Steffen
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
 * @defgroup pwg_attr pwg_attr
 * @{ @ingroup libimcv
 */

#ifndef PWG_ATTR_H_
#define PWG_ATTR_H_

#include <pa_tnc/pa_tnc_attr.h>
#include <library.h>

typedef enum pwg_attr_t pwg_attr_t;

/**
 * PWG HCD IF-M Attributes (Hardcopy Device Health Assessment TNC Binding)
 */
enum pwg_attr_t {
	PWG_HCD_ATTRS_NATURAL_LANG =          0x00000001, /*   1 */
	PWG_HCD_MACHINE_TYPE_MODEL =          0x00000002, /*   2 */
	PWG_HCD_VENDOR_NAME =                 0x00000003, /*   3 */
	PWG_HCD_VENDOR_SMI_CODE =             0x00000004, /*   4 */
	PWG_HCD_DEFAULT_PWD_ENABLED =         0x00000014, /*  20 */
	PWG_HCD_FIREWALL_SETTING =            0x00000015, /*  21 */
	PWG_HCD_FORWARDING_ENABLED =          0x00000016, /*  22 */
	PWG_HCD_PSTN_FAX_ENABLED =            0x00000028, /*  40 */
	PWG_HCD_TIME_SOURCE =                 0x00000032, /*  50 ??? */
	PWG_HCD_FIRMWARE_NAME =               0x0000003C, /*  60 */
	PWG_HCD_FIRMWARE_PATCHES =            0x0000003D, /*  61 */
	PWG_HCD_FIRMWARE_STRING_VERSION =     0x0000003E, /*  62 */
	PWG_HCD_FIRMWARE_VERSION =            0x0000003F, /*  63 */
	PWG_HCD_RESIDENT_APP_NAME =           0x00000050, /*  80 */
	PWG_HCD_RESIDENT_APP_PATCHES =        0x00000051, /*  81 */
	PWG_HCD_RESIDENT_APP_STRING_VERSION = 0x00000052, /*  82 */
	PWG_HCD_RESIDENT_APP_VERSION =        0x00000053, /*  83 */
	PWG_HCD_USER_APP_NAME =               0x00000064, /* 100 */
	PWG_HCD_USER_APP_PATCHES =            0x00000065, /* 101 */
	PWG_HCD_USER_APP_STRING_VERSION =     0x00000066, /* 102 */
	PWG_HCD_USER_APP_VERSION =            0x00000067, /* 103 */
	PWG_HCD_USER_APP_ENABLED =            0x00000068, /* 104 */
	PWG_HCD_USER_APP_PERSIST_ENABLED =    0x00000069, /* 105 */
	PWG_HCD_CERTIFICATION_STATE =         0x000000C8, /* 200 */
	PWG_HCD_CONFIGURATION_STATE =         0x000000C9, /* 201 */
};

/**
 * enum name for pwg_attr_t.
 */
extern enum_name_t *pwg_attr_names;

/**
 * Create a TCG PA-TNC attribute from data
 *
 * @param type				attribute type
 * @param length			attribute length
 * @param value				attribute value or segment
 */
pa_tnc_attr_t* pwg_attr_create_from_data(uint32_t type, size_t length,
										 chunk_t value);

#endif /** PWG_ATTR_H_ @}*/
