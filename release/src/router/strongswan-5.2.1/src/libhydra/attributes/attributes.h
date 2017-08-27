/*
 * Copyright (C) 2005-2006 Martin Willi
 * Copyright (C) 2005 Jan Hutter
 * Hochschule fuer Technik Rapperswil
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
 * @defgroup attributes_g attributes
 * @{ @ingroup attributes
 */

#ifndef ATTRIBUTES_H_
#define ATTRIBUTES_H_

typedef enum configuration_attribute_type_t configuration_attribute_type_t;

#include <utils/utils.h>

/**
 * Type of the attribute, as in IKEv2 RFC 3.15.1 or IKEv1 ModeConfig.
 */
enum configuration_attribute_type_t {
	INTERNAL_IP4_ADDRESS    = 1,
	INTERNAL_IP4_NETMASK    = 2,
	INTERNAL_IP4_DNS        = 3,
	INTERNAL_IP4_NBNS       = 4,
	INTERNAL_ADDRESS_EXPIRY = 5,
	INTERNAL_IP4_DHCP       = 6,
	APPLICATION_VERSION     = 7,
	INTERNAL_IP6_ADDRESS    = 8,
	INTERNAL_IP6_NETMASK    = 9,
	INTERNAL_IP6_DNS        = 10,
	INTERNAL_IP6_NBNS       = 11,
	INTERNAL_IP6_DHCP       = 12,
	INTERNAL_IP4_SUBNET     = 13,
	SUPPORTED_ATTRIBUTES    = 14,
	INTERNAL_IP6_SUBNET     = 15,
	MIP6_HOME_PREFIX        = 16,
	INTERNAL_IP6_LINK       = 17,
	INTERNAL_IP6_PREFIX     = 18,
	HOME_AGENT_ADDRESS		= 19,
	/* XAUTH attributes */
	XAUTH_TYPE              = 16520,
	XAUTH_USER_NAME         = 16521,
	XAUTH_USER_PASSWORD     = 16522,
	XAUTH_PASSCODE          = 16523,
	XAUTH_MESSAGE           = 16524,
	XAUTH_CHALLENGE         = 16525,
	XAUTH_DOMAIN            = 16526,
	XAUTH_STATUS            = 16527,
	XAUTH_NEXT_PIN          = 16528,
	XAUTH_ANSWER            = 16529,
	/* proprietary Microsoft attributes */
	INTERNAL_IP4_SERVER     = 23456,
	INTERNAL_IP6_SERVER     = 23457,
	/* proprietary Cisco Unity attributes */
	UNITY_BANNER            = 28672,
	UNITY_SAVE_PASSWD       = 28673,
	UNITY_DEF_DOMAIN        = 28674,
	UNITY_SPLITDNS_NAME     = 28675,
	UNITY_SPLIT_INCLUDE     = 28676,
	UNITY_NATT_PORT         = 28677,
	UNITY_LOCAL_LAN         = 28678,
	UNITY_PFS               = 28679,
	UNITY_FW_TYPE           = 28680,
	UNITY_BACKUP_SERVERS    = 28681,
	UNITY_DDNS_HOSTNAME     = 28682
};

/**
 * enum names for configuration_attribute_type_t.
 */
extern enum_name_t *configuration_attribute_type_names;

/**
 * Short enum names for configuration_attribute_type_t.
 */
extern enum_name_t *configuration_attribute_type_short_names;


#endif /** ATTRIBUTES_H_ @}*/
