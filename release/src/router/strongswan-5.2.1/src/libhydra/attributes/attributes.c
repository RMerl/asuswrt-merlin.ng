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


#include "attributes.h"

ENUM_BEGIN(configuration_attribute_type_names, INTERNAL_IP4_ADDRESS, HOME_AGENT_ADDRESS,
	"INTERNAL_IP4_ADDRESS",
	"INTERNAL_IP4_NETMASK",
	"INTERNAL_IP4_DNS",
	"INTERNAL_IP4_NBNS",
	"INTERNAL_ADDRESS_EXPIRY",
	"INTERNAL_IP4_DHCP",
	"APPLICATION_VERSION",
	"INTERNAL_IP6_ADDRESS",
	"INTERNAL_IP6_NETMASK",
	"INTERNAL_IP6_DNS",
	"INTERNAL_IP6_NBNS",
	"INTERNAL_IP6_DHCP",
	"INTERNAL_IP4_SUBNET",
	"SUPPORTED_ATTRIBUTES",
	"INTERNAL_IP6_SUBNET",
	"MIP6_HOME_PREFIX",
	"INTERNAL_IP6_LINK",
	"INTERNAL_IP6_PREFIX",
	"HOME_AGENT_ADDRESS");
ENUM_NEXT(configuration_attribute_type_names, XAUTH_TYPE, XAUTH_ANSWER, HOME_AGENT_ADDRESS,
	"XAUTH_TYPE",
	"XAUTH_USER_NAME",
	"XAUTH_USER_PASSWORD",
	"XAUTH_PASSCODE",
	"XAUTH_MESSAGE",
	"XAUTH_CHALLENGE",
	"XAUTH_DOMAIN",
	"XAUTH_STATUS",
	"XAUTH_NEXT_PIN",
	"XAUTH_ANSWER");
ENUM_NEXT(configuration_attribute_type_names, INTERNAL_IP4_SERVER, INTERNAL_IP6_SERVER, XAUTH_ANSWER,
	"INTERNAL_IP4_SERVER",
	"INTERNAL_IP6_SERVER");
ENUM_NEXT(configuration_attribute_type_names, UNITY_BANNER, UNITY_DDNS_HOSTNAME, INTERNAL_IP6_SERVER,
	"UNITY_BANNER",
	"UNITY_SAVE_PASSWD",
	"UNITY_DEF_DOMAIN",
	"UNITY_SPLITDNS_NAME",
	"UNITY_SPLIT_INCLUDE",
	"UNITY_NATT_PORT",
	"UNITY_LOCAL_LAN",
	"UNITY_PFS",
	"UNITY_FW_TYPE",
	"UNITY_BACKUP_SERVERS",
	"UNITY_DDNS_HOSTNAME");
ENUM_END(configuration_attribute_type_names, UNITY_DDNS_HOSTNAME);

ENUM_BEGIN(configuration_attribute_type_short_names, INTERNAL_IP4_ADDRESS, HOME_AGENT_ADDRESS,
	"ADDR",
	"MASK",
	"DNS",
	"NBNS",
	"EXP",
	"DHCP",
	"VER",
	"ADDR6",
	"MASK6",
	"DNS6",
	"NBNS6",
	"DHCP6",
	"SUBNET",
	"SUP",
	"SUBNET6",
	"MIP6HPFX",
	"LINK6",
	"PFX6",
	"HOA");
ENUM_NEXT(configuration_attribute_type_short_names, XAUTH_TYPE, XAUTH_ANSWER, HOME_AGENT_ADDRESS,
	"X_TYPE",
	"X_USER",
	"X_PWD",
	"X_CODE",
	"X_MSG",
	"X_CHALL",
	"X_DOMAIN",
	"X_STATUS",
	"X_PIN",
	"X_ANSWER");
ENUM_NEXT(configuration_attribute_type_short_names, INTERNAL_IP4_SERVER, INTERNAL_IP6_SERVER, XAUTH_ANSWER,
	"SRV",
	"SRV6");
ENUM_NEXT(configuration_attribute_type_short_names, UNITY_BANNER, UNITY_DDNS_HOSTNAME, INTERNAL_IP6_SERVER,
	"U_BANNER",
	"U_SAVEPWD",
	"U_DEFDOM",
	"U_SPLITDNS",
	"U_SPLITINC",
	"U_NATTPORT",
	"U_LOCALLAN",
	"U_PFS",
	"U_FWTYPE",
	"U_BKPSRV",
	"U_DDNSHOST");
ENUM_END(configuration_attribute_type_short_names, UNITY_DDNS_HOSTNAME);
